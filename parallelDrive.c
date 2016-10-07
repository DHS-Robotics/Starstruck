/*///////////////  INSTRUCTIONS  /////////////////
	1. Save this file, timer.c, coreIncludes.c, and motorGroup.c in the same directory as your code
	2. Include this line near the top of your code:
			| #include "paralleldrive.c"
	3. To create drive, include the following lines in your code:
			| parallel_drive driveName;
			| initializeDrive(driveName);
			| setLeftMotors(driveName, numLeftMotors, left1, left2, left3, etc.);
			| setRightMotors(driveName, numRightMotors, right1, right2, right3, etc.);
	   Any valid variable name can be substituted for driveName
	   numLeftMotors and numRightMotors are the number of motors on that side of the drive (Maximum of 6)
	   Motor names (right1, right2, left1, left2, etc...) should correspond to the names assigned in motor setup.
	   The optional arguments of initializeDrive can be used to further configure the drive
	   	e.g. a user creating a drive with ramping and quadratic input mapping would substitute the second line of code with:
	   	| initializeDrive(driveName, true, 20, 10, 2);
	4. Whenever the drive should be updated (probably once every input cycle) include the following line of code
			| driveRuntime(driveName);
		Where driveName is the same as in the previous step
	5. To explicitly set drive power, use setDrivePower(driveName, leftPower, rightPower), setRightPower(driveName, power), or setLeftPower(driveName, power)
	6. To attach sensors, call attachGyro(driveName, gyro), attachEncoderL(driveName, leftEncoder), or attachEncoderR(driveName, rightEncoder), where gyro, leftEncoder, and rightEncoder are the names assigned in sensor setup
		The attachEncoder functions also accept wheelDiameter and gearRatio arguments. These default to 3.25" and 1, and are used to convert encoder values to distance ones.
	7. To explicitly control how encoders are used for distance measurement, call setEncoderConfig(driveName, config) where config is NONE, LEFT, RIGHT, or AVERAGE
	8. To access a sensor value, call encoderVal(driveName), encoderVal_L(driveName), encoderVal_R(driveName), gyroVal(driveName), or absAngle(driveName).
		Encoder values are converted into distance ones unless optional rawValue argument is set to true.
		The optional angleType argument accepts DEGREES, RADIANS, or RAW, and controls the output format of gyroVal() and absAngle().
		absAngle() returns the absolute gyro reading, which stays constant even when the gyro is reset.
	9. To reset a sensor, call resetEncoders(driveName), resetLeft(driveName), resetRight(driveName), resetGyro(driveName), or resetAbsAngle(driveName).
		To set a sensor to a value other than zero, use the optional resetVal argument. To specify the input format for the gyro, use the angleType argument.
	10.To set the robot's position, call setRobotPosition(driveName, x, y, theta);
	11.To have the robot's position automatically updated (very experimental feature), include the following line of code in the main loop
			| updatePosition(driveName);
		This function returns a robotPosition object.
	12.To auto-calculate a drive's width, write a test program creating a parallel drive with a gyro and at least one encoder.
		The function calculateWidth(driveName) will cause the robot to turn several times, using gyro and encoder values to calculate the width of the drive-> It returns a float.
*/

#include "coreIncludes.c"
#include "timer.c"
#include "motorGroup.c"

enum encoderConfig { UNASSIGNED, LEFT, RIGHT, AVERAGE };
enum gyroCorrectionType { NONE, MEDIUM, FULL };

typedef struct {
	float x;
	float y;
	float theta;
} robotPosition;

typedef struct {
	motorGroup leftDrive, rightDrive;
	robotPosition position; //(x, y) coordinates and orientation of robot
	float width; //width of drive in inches (wheel well to wheel well). Used to track position.
	//internal variables
	//position tracking
	long posLastUpdated;
	int minSampleTime;
	gyroCorrectionType gyroCorrection;
	//associated sensors
	encoderConfig encoderConfig;
	bool hasGyro;
	bool gyroReversed;
	float angleOffset; //amount added to gyro values to obtain absolute angle
	float encCoeff; //coefficients used to translate encoder values to distance traveled
	tSensors gyro;
} parallel_drive;


void initializeDrive(parallel_drive *drive, bool isRamped=false, int maxAcc100ms=20, int deadband=10, float powMap=1, float maxPow=127, float initialX=0, float initialY=0, float initialTheta=PI/2, float width=16, int minSampleTime=50, TVexJoysticks leftInput=Ch3, TVexJoysticks rightInput=Ch2) {
	//initialize drive variables
	configureJoystickInput(drive->leftDrive, leftInput, deadband, isRamped, maxAcc100ms, powMap, maxPow);
	configureJoystickInput(drive->rightDrive, rightInput, deadband, isRamped, maxAcc100ms, powMap, maxPow);
	drive->position.x = initialX;
	drive->position.y = initialY;
	drive->position.theta = initialTheta;
	drive->width = width;
	drive->minSampleTime = minSampleTime;
	drive->gyroCorrection = NONE;
	drive->posLastUpdated = resetTimer();
}

void setDriveMotors(parallel_drive *drive, int numMotors, tMotor motor1, tMotor motor2=port1, tMotor motor3=port1, tMotor motor4=port1, tMotor motor5=port1, tMotor motor6=port1, tMotor motor7=port1, tMotor motor8=port1, tMotor motor9=port1, tMotor motor10=port1, tMotor motor11=port1, tMotor motor12=port1) {
	tMotor motors[12] = { motor1, motor2, motor3, motor4, motor5, motor6, motor7, motor8, motor9, motor10, motor11, motor12 };

	int numSideMotors = numMotors / 2;

	initializeGroup(drive->leftDrive, numSideMotors, motors[0], motors[1], motors[2], motors[3], motors[4], motors[5]);
	initializeGroup(drive->rightDrive, numSideMotors, motors[numSideMotors], motors[numSideMotors + 1], motors[numSideMotors + 2], motors[numSideMotors + 3], motors[numSideMotors + 4], motors[numSideMotors + 5]);
}


//sensor setup region
void updateEncoderConfig(parallel_drive *drive) {
	if (drive->leftDrive.hasEncoder) {
		if (drive->rightDrive.hasEncoder) {
			drive->encoderConfig = AVERAGE;
		} else {
			drive->encoderConfig = LEFT;
		}
	} else {
		drive->encoderConfig = RIGHT; //safe assuming nothig but attachEncoder functions call this
	}
}

void attachEncoder(parallel_drive *drive, tSensors encoder, encoderConfig side, bool reversed=false, float wheelDiameter=3.25, float gearRatio=1) {
	if (side == LEFT) {
		addSensor(drive->leftDrive, encoder, reversed);
	} else {
		addSensor(drive->rightDrive, encoder, reversed);
	}

	drive->encCoeff = PI * wheelDiameter * gearRatio / 360;
	updateEncoderConfig(drive);
}

void attachGyro(parallel_drive *drive, tSensors gyro, bool reversed=true, gyroCorrectionType correction=MEDIUM, bool setAbsAngle=true) {
	drive->gyro = gyro;
	drive->hasGyro = true;
	drive->gyroReversed = reversed;
	drive->gyroCorrection = correction;

	if (setAbsAngle) drive->angleOffset = drive->position.theta - SensorValue[gyro];
}

void setEncoderConfig(parallel_drive *drive, encoderConfig config) {
	drive->encoderConfig = config;
}
//end sensor setup region


//sensor access region
float driveEncoderVal(parallel_drive *drive, encoderConfig side=UNASSIGNED, bool rawValue=false, bool absolute=true) {
	if (side == UNASSIGNED) {
		side = drive->encoderConfig;
	}

	if (side == AVERAGE) {
		if (absolute) {
			return (abs(driveEncoderVal(drive, LEFT, rawValue)) + abs(driveEncoderVal(drive, RIGHT, rawValue))) / 2;
		} else {
			return (driveEncoderVal(drive, LEFT, rawValue) + driveEncoderVal(drive, RIGHT, rawValue)) / 2;
		}
	} else if (side == LEFT) {
		return encoderVal(drive->leftDrive) * (rawValue ? 1 : drive->encCoeff);
	} else if (side == RIGHT) {
		return encoderVal(drive->rightDrive) * (rawValue ? 1 : drive->encCoeff);
	}

	return 0;
}

void resetLeft(parallel_drive *drive, int resetVal=0) {
	resetEncoder(drive->leftDrive, resetVal);
}

void resetRight(parallel_drive *drive, int resetVal=0) {
	resetEncoder(drive->rightDrive, resetVal);
}

void resetDriveEncoders(parallel_drive *drive, int resetVal=0) {
	resetLeft(drive, resetVal);
	resetRight(drive, resetVal);
}

float gyroVal(parallel_drive *drive, angleType format=DEGREES) {
	return convertAngle(SensorValue[drive->gyro] * (drive->gyroReversed ? 1 : -1), format);
}

void resetGyro(parallel_drive *drive, float resetVal=0, angleType format=DEGREES, bool setAbsAngle=true) {
	if (setAbsAngle) drive->angleOffset += gyroVal(drive);

	SensorValue[drive->gyro] = (int)(convertAngle(resetVal, RAW, format));

	if (setAbsAngle) drive->angleOffset -= gyroVal(drive); //I could include this two lines up, except this function doesn't usually work as expected
}

float absAngle(parallel_drive *drive, angleType format=DEGREES) {
	return gyroVal(drive, format) + convertAngle(drive->angleOffset, format);
}

void resetAbsAngle(parallel_drive *drive, float angle=0, angleType format=DEGREES) {
	drive->angleOffset = convertAngle(angle, RAW, format) - gyroVal(drive, RAW);
}
//end sensor access region


//position tracking region
void setRobotPosition(parallel_drive *drive, float x, float y, float theta, bool setAbsAngle=true) {
	drive->position.x = x;
	drive->position.y = y;
	drive->position.theta = theta;

	if (setAbsAngle) resetAbsAngle(drive, theta, RADIANS);
}

void updatePosition(parallel_drive *drive) {
	if (time(drive->posLastUpdated) >= drive->minSampleTime) {
		float leftDist = driveEncoderVal(drive, LEFT);
		float rightDist = driveEncoderVal(drive, RIGHT);
		float angle = absAngle(drive, RADIANS);
		resetDriveEncoders(drive);

		drive->posLastUpdated = resetTimer();

		if (drive->gyroCorrection == FULL && rightDist+leftDist != 0) {
			float deltaT = angle - drive->position.theta;
			float correctionFactor = (rightDist - leftDist - drive->width*deltaT) / (rightDist + leftDist);
			leftDist *= 1 + correctionFactor;
			rightDist *= 1 - correctionFactor;
		}

		if (rightDist != leftDist && leftDist != 0) {
			float r = drive->width/(rightDist/leftDist - 1.0) + drive->width/2;
			float phi = (rightDist - leftDist) / drive->width;

			drive->position.x += r * (sin(drive->position.theta + phi) - sin(drive->position.theta));
			drive->position.y += r * (cos(drive->position.theta) - cos(drive->position.theta + phi));
			drive->position.theta = (drive->gyroCorrection==NONE ? drive->position.theta+phi : angle);
		} else {
			drive->position.x += leftDist * cos(drive->position.theta);
			drive->position.y += leftDist * sin(drive->position.theta);
		}
	}
}
//end position tracking region


//set drive power region
void setLeftPower (parallel_drive *drive, int power) {
	setPower(drive->leftDrive, power);
}

void setRightPower (parallel_drive *drive, int power) {
	setPower(drive->rightDrive, power);
}

void setDrivePower (parallel_drive *drive, int left, int right) {
	setLeftPower(drive, left);
	setRightPower(drive, right);
}
//end set drive power region


//misc
float calculateWidth(parallel_drive *drive, int duration=10000, int sampleTime=200, int power=80, int reverseDelay=750) {
	if (drive->hasGyro && drive->encoderConfig != UNASSIGNED) {
		long timer;
		float totalWidth = 0;
		int samples = 0;

		for (int i=1; i>=0; i--) { //turn both directions
			setDrivePower(drive, (2*i-1) * power, (1-2*i) * power); //formula to reverse direction
			wait1Msec(reverseDelay * i); //delay second time only
			timer = resetTimer();

			while (time(timer) < (duration-reverseDelay)/2) {
				resetDriveEncoders(drive);
				resetGyro(drive);
				wait1Msec(sampleTime);

				totalWidth += driveEncoderVal(drive) * 3600 / (PI * abs(gyroVal(drive, RAW)));
				samples++;
			}
		}
		return totalWidth / samples;
	} else {
		return 0;
	}
}
//end misc


void driveRuntime(parallel_drive *drive) {
	takeInput(drive->leftDrive);
	takeInput(drive->rightDrive);
}
