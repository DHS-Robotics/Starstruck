#pragma config(Sensor, in2,    hyro,           sensorGyro)
#pragma config(Sensor, in3,    liftPot,        sensorPotentiometer)
#pragma config(Sensor, in5,    modePot,        sensorPotentiometer)
#pragma config(Sensor, in6,    sidePot,        sensorPotentiometer)
#pragma config(Sensor, in8,    clawPot,        sensorPotentiometer)
#pragma config(Sensor, dgtl1,  rightEnc,       sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  leftEnc,        sensorQuadEncoder)
#pragma config(Motor,  port1,           rd1,           tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           rd2,           tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           lift1,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           lift2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port5,           lift3,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           clawMotor,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           lift4,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           lift5,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           ld1,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          ld2,           tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX2)
#pragma competitionControl(Competition)
#include "Vex_Competition_Includes.c"

//#region includes
#include "..\Includes\motorGroup.c"
#include "..\Includes\parallelDrive.c"
#include "..\Includes\pd_autoMove.c"
#include "..\Includes\timer.c"
//#endregion

//#region buttons
#define autoDumpOnBtn Btn8U //claw
#define autoDumpOffBtn Btn8D
#define openClawBtn Btn6U
#define closeClawBtn Btn6D
#define liftUpBtn Btn5U //lift
#define liftDownBtn Btn5D
//#endregion

//#region positions
#define liftBottom 1190 //lift
#define liftMiddle 1420
#define liftTop 1700 //1885
#define liftThrowPos 2260
#define liftMax 2500 //2700
#define clawClosedPos 1450 //claw
#define clawOpenPos 1800
#define clawMax 2150 //2400
//#endregion

//#region constants
#define liftStillSpeed 10 //still speeds
#define clawStillSpeed 15
//#endregion

//#region globals
bool clawOpen = true;
bool liftDown = true;
bool autoDumping = true;
int autoSign; //for autonomous, positive if robot is left of pillow

motorGroup lift;
motorGroup claw;
//#endregion

void pre_auton() {
	bStopTasksBetweenModes = true;

	//configure drive
	initializeDrive(drive);
	setDriveMotors(drive, 4, ld1, ld2, rd1, rd2);
	attachEncoder(drive, leftEnc, LEFT);
	attachEncoder(drive, rightEnc, RIGHT, false, 3.25);
	attachGyro(drive, hyro);

	//configure lift
	initializeGroup(lift, 5, lift1, lift2, lift3, lift4, lift5);
	configureButtonInput(lift, liftUpBtn, liftDownBtn, liftStillSpeed);
	addSensor(lift, liftPot);

	//configure claw
	initializeGroup(claw, 1, clawMotor);
	addSensor(claw, clawPot);
}

//#region lift
void liftControl() {
	lift.stillSpeed = liftStillSpeed * (potentiometerVal(lift)<liftMiddle ? -1 : 1);

	takeInput(lift);
}
//#endregion

//#region claw
void clawControl() {
	if (getPosition(lift)>liftThrowPos && getPosition(claw)<clawOpenPos && autoDumping) {
		setPower(claw, 127);
		clawOpen = true;
	} else	if (vexRT[openClawBtn] == 1) {
		setPower(claw, 127);
		clawOpen = true;
	} else if (vexRT[closeClawBtn] == 1) {
		setPower(claw, -127);
		clawOpen = false;
	} else {
		setPower(claw, clawStillSpeed * (clawOpen ? 1 : -1));
	}

	if (vexRT[autoDumpOnBtn] == 1)
		autoDumping = true;
	else if (vexRT[autoDumpOffBtn] == 1)
		autoDumping = false;
}
//#endregion

//#region autonomous
task maneuvers() {
	while (true) {
		executeManeuver(claw);
		executeManeuver(lift);
	}
}

void setClawStateManeuver(bool open, int power=127) {
	if (open) {
		createManeuver(claw, clawOpenPos, clawStillSpeed, power);
	} else {
		createManeuver(claw, clawClosedPos, -clawStillSpeed, power);
	}

	clawOpen = open;
}

void openClaw(bool stillSpeed=true, int power=127) {
	goToPosition(claw, clawOpenPos, (stillSpeed ? clawStillSpeed : 0), power);

	clawOpen = true;
}

void closeClaw(bool stillSpeed=true, int timeout=500, int power=127, int minSpeed=150, int sampleTime=100) { //minSpeed in encoder/potentiometer values per second
	int minDiffPerSample = minSpeed * sampleTime / 1000;
	int prevPos = getPosition(claw);
	int newPos;
	long clawTimer = resetTimer();

	setPower(claw, -127);

	while (time(clawTimer)<timeout || getPosition(claw)>clawOpenPos) {
		wait1Msec(sampleTime);
		newPos = getPosition(claw);

		if (newPos - prevPos > minDiffPerSample) clawTimer = resetTimer();

		prevPos = newPos;
	}

	setPower(claw, (stillSpeed ? -clawStillSpeed : 0));

	clawOpen = false;
}

void hyperExtendClaw(bool stillSpeed=true) {
	goToPosition(claw, clawMax, (stillSpeed ? clawStillSpeed : 0));

	clawOpen = true;
}

void setLiftStateManeuver(bool top) {
	if (top) {
		createManeuver(lift, liftTop, liftStillSpeed);
	} else {
		createManeuver(lift, liftBottom, liftStillSpeed);
	}

	liftDown = !top;
}

void grabNdump(int delayDuration, int distance=30, int closeTimeout=500) {
	wait1Msec(delayDuration); //wait for objects to be dropped
	closeClaw(true, closeTimeout);
	createManeuver(lift, liftMax, -liftStillSpeed); //lift to top
	driveStraight(-distance, true); //drive to fence
	while (getPosition(lift) < liftThrowPos);
	setClawStateManeuver(true);
	while (driveData.isDriving || lift.maneuverExecuting || claw.maneuverExecuting);
}

void driveToWall(int distance=25) {
	goToPosition(lift, liftBottom, -liftStillSpeed);
	driveStraight(distance);
}

void initialPillow() {
	setPower(lift, -liftStillSpeed);

	//open claw, drive away from wall, and lift up a little bit
	setClawStateManeuver(true/*, 50*/);
	driveStraight(7, true);
	while(driveData.isDriving || lift.maneuverExecuting);

	//drive to central pillow
	turn(autoSign * -57, true);
	while(turnData.isTurning || claw.maneuverExecuting);
	driveStraight(14);

	closeClaw(); //clamp pillow
}

task skillz() {
	setClawStateManeuver(true);
	driveStraight(-10, true);
	while (claw.maneuverExecuting || driveData.isDriving);

	grabNdump(2000);
	driveToWall();

	grabNdump(500);
	driveToWall();

	grabNdump(500);

	//get pillow in center of field
	setLiftStateManeuver(false);
	//createManeuver(claw, clawOpenPos, clawStillSpeed);
	while (lift.maneuverExecuting || claw.maneuverExecuting);
	turn(-33, false, 40, 127, -10);
	driveStraight(20);
	closeClaw(); //grab pillow

	//dump pillow
	createManeuver(lift, liftMax, -liftStillSpeed, 90);
	turn(49, true);
	while (turnData.isTurning);
	driveStraight(-13, true);
	while (getPosition(lift) < liftThrowPos);
	setClawStateManeuver(true);
	while (driveData.isDriving || lift.maneuverExecuting || claw.maneuverExecuting);

	//grab and dump jacks
	driveStraight(25, true);
	createManeuver(claw, clawMax, clawStillSpeed);
	setLiftStateManeuver(false);
	while (driveData.isDriving || claw.maneuverExecuting);
	grabNdump(0, 30, 750);

	//go to pillow
	setLiftStateManeuver(false);
	turn(-18, true);
	while (lift.maneuverExecuting || turnData.isTurning);
	driveStraight(30);
	closeClaw();

	//dump pillow
	goToPosition(lift, liftMiddle, liftStillSpeed);
	turn(47);
	//createManeuver(lift, liftMax, -liftStillSpeed, 90);
	driveStraight(-30, true);
	while (driveData.totalDist < 10);
	createManeuver(lift, liftMax, -liftStillSpeed);
	while (getPosition(lift) < liftThrowPos);
	setClawStateManeuver(true);
	while (driveData.isDriving || lift.maneuverExecuting || claw.maneuverExecuting);

	//pick up central jacks
	createManeuver(lift, liftMiddle, liftStillSpeed);
	turn(70, true);
	while (turnData.isTurning || lift.maneuverExecuting);
	createManeuver(lift, liftBottom, -liftStillSpeed);
	driveStraight(20, true);
	while (driveData.isDriving || lift.maneuverExecuting);
	closeClaw();

	//dump jacks
	createManeuver(lift, liftMax, liftStillSpeed);
	turn(-60, true);
	while (getPosition(lift) < liftThrowPos);
	setClawStateManeuver(true);
	while (claw.maneuverExecuting || lift.maneuverExecuting || turnData.isTurning);
	goToPosition(lift, liftBottom);
}

task pillowAuton() {
	initialPillow();

	//go to fence and lift up
	setLiftStateManeuver(true);
	driveStraight(10.5, true);
	while (driveData.isDriving);
	turn(autoSign * 57, true, 40, 80, -10); //turn to face fence
	while (turnData.isTurning);
	driveStraight(20, true); // drive up to wall
	while (driveData.isDriving || lift.maneuverExecuting);

	openClaw(); //release pillow
	wait1Msec(600); //wait for pillow to fall
	closeClaw();
	driveStraight(-4); //back up
	hyperExtendClaw();

	//push jacks over
 	driveStraight(6);
 	closeClaw();

 	createManeuver(claw, clawMax, clawStillSpeed);

 	//drive to other wall and lift down
 	driveStraight(-10, true);
 	while (driveData.isDriving);
 	turn(autoSign * 65, true, 40, 127, -20);
 	while (turnData.isTurning || claw.maneuverExecuting);
 	createManeuver(lift, liftMiddle+100, liftStillSpeed);
 	driveStraight(35, true);
 	while (driveData.isDriving || lift.maneuverExecuting);
 	turn(autoSign * -65, false, 40, 120, -40);
 	driveStraight(10);

 	goToPosition(lift, liftTop+75); //push jacks over
 	driveStraight(5);
 	closeClaw();
}

task dumpyAuton() {
	initialPillow();

	goToPosition(lift, liftMiddle, liftStillSpeed);

	driveStraight(7);
	wait1Msec(500);
	turn(autoSign * -90, false, 45, 120, -20);
	driveStraight(-17, true);
	createManeuver(lift, liftMax, liftStillSpeed);
	while (potentiometerVal(lift) < liftThrowPos);
	createManeuver(claw, clawMax-75, clawStillSpeed);
	while (driveData.isDriving || lift.maneuverExecuting || claw.maneuverExecuting);
	wait1Msec(250);

	driveToWall();
	grabNdump(0, 30, 750);
	driveToWall();
	grabNdump(0, 30, 750);
	goToPosition(lift, liftBottom, -liftStillSpeed);
}

task oneSideAuton() {
	createManeuver(claw, clawMax, clawStillSpeed); //open claw
	createManeuver(lift, liftTop-450, liftStillSpeed); //lift to near top
	driveStraight(5, true); //drive away from wall
	while(driveData.isDriving);

	turn(-30, true);
	while (turnData.isTurning);

	driveStraight(18, true);
	while (driveData.isDriving || claw.maneuverExecuting);

	turn(37, true); //turn toward wall
	while (turnData.isTurning || lift.maneuverExecuting);

	//knock off jacks
	driveStraight(42);
	goToPosition(lift, 1250, liftStillSpeed);
	closeClaw();
	wait1Msec(2500);

	//go to back jacks
	setClawStateManeuver(true);
	turn(120, true, 40, 127, -10);
	while (turnData.isTurning);
	setLiftStateManeuver(false);
	driveStraight(46, true);
	while (driveData.isDriving || lift.maneuverExecuting || claw.maneuverExecuting);
	closeClaw();
	wait1Msec(3500);

	//dump
	createManeuver(lift, liftMax, -liftStillSpeed);
	turn(50, true);
	while (turnData.isTurning);
	driveStraight(-30, true);
	while(driveData.isDriving || lift.maneuverExecuting);
	openClaw();
	goToPosition(lift, liftBottom);
}

task autonomous() {
	lift.maneuverExecuting = false;
	claw.maneuverExecuting = false;
	startTask(maneuvers);

	int sidePos = SensorValue[sidePot];

	autoSign = (sidePos < 1900) ? 1 : -1;

	//start appropriate autonomous task
	if (sidePos>1030 && sidePos<2585) {
		startTask(skillz);
	} else if (SensorValue[modePot] > 2670) {
		startTask(pillowAuton);
	} else if (SensorValue[modePot] > 1275) {
		startTask(dumpyAuton);
	}
}
//#endregion

task usercontrol() {
	while (true) {
		driveRuntime(drive);

		liftControl();

		clawControl();
	}
}
