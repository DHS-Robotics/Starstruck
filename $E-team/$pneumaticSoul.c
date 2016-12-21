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
#pragma config(Motor,  port10,          ld2,					 tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//#region setup
#pragma platform(VEX2)
#pragma competitionControl(Competition)
#include "Vex_Competition_Includes.c"
#include "..\Includes\pd_autoMove.c"
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
#define liftTop 1700
#define liftThrowPos 2260
#define liftMax 2500
#define clawClosedPos 1450 //claw
#define clawOpenPos 1800
#define clawMax 2150
//#endregion

//#region constants
#define liftStillSpeed 10 //still speeds
#define clawStillSpeed 15
#define fenceToWallDist 25 //distances
//#endregion

//#region globals
bool clawOpen = true;
bool liftDown = true;
bool autoDumping = true;
int autoSign; //for autonomous, positive if robot is left of pillow

motorGroup lift;
motorGroup claw;
//#endregion

//#region enums
enum clawState { CLOSED, OPEN, HYPEREXTENDED };
enum liftState { BOTTOM, MIDDLE, TOP, THROW, MAX };
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
	if (vexRT[openClawBtn] == 1) {
		setPower(claw, 127);
		clawOpen = true;
	} else if (vexRT[closeClawBtn] == 1) {
		setPower(claw, -127);
		clawOpen = false;
	} else if (getPosition(lift)>liftThrowPos && getPosition(claw)<clawOpenPos && autoDumping) {
		setPower(claw, 127);
		clawOpen = true;
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
void maneuvers() {
	executeManeuver(claw);
	executeManeuver(lift);
}

void createClawManeuver(clawState state, int power=127) {
	switch (state) {
		case CLOSED:
			createManeuver(claw, clawClosedPos, -clawStillSpeed, power);
			clawOpen = false;
			break;
		case OPEN:
			createManeuver(claw, clawOpenPos, clawStillSpeed, power);
			clawOpen = true;
			break;
		case HYPEREXTENDED:
			createManeuver(claw, clawMax, clawStillSpeed, power);
			clawOpen = true;
			break;
	}
}

void openClaw(bool stillSpeed=true, int power=127) {
	goToPosition(claw, clawOpenPos, (stillSpeed ? clawStillSpeed : 0), power);

	clawOpen = true;
}

void closeClaw(int timeout=500, int power=127, bool stillSpeed=true, int minSpeed=150, int sampleTime=100) { //minSpeed in encoder/potentiometer values per second
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

void createLiftManeuver(liftState state, int power=127) {
	liftDown = false;

	switch (state) {
		case BOTTOM:
			createManeuver(lift, liftBottom, -liftStillSpeed, power);
			liftDown = true;
			break;
		case MIDDLE:
			createManeuver(lift, liftMiddle, liftStillSpeed, power);
			break;
		case TOP:
			createManeuver(lift, liftTop, liftStillSpeed, power);
			break;
		case THROW:
			createManeuver(lift, liftThrowPos, liftStillSpeed, power);
			break;
		case MAX:
			createManeuver(lift, liftMax, liftStillSpeed, power);
			break;
	}
}

void liftTo(liftState state, int power=127) {
	liftDown = false;

	switch (state) {
		case BOTTOM:
			goToPosition(lift, liftBottom, -liftStillSpeed, power);
			liftDown = true;
			break;
		case MIDDLE:
			goToPosition(lift, liftMiddle, liftStillSpeed, power);
			break;
		case TOP:
			goToPosition(lift, liftTop, liftStillSpeed, power);
			break;
		case THROW:
			goToPosition(lift, liftThrowPos, liftStillSpeed, power);
			break;
		case MAX:
			goToPosition(lift, liftMax, liftStillSpeed, power);
			break;
	}
}

void turnDriveDump (int angle, int dist, int distCutoff=0, float turnConst1=defTurnFloats[0], float turnConst2=defTurnFloats[1], float turnConst3=defTurnFloats[2]) {
	if (angle != 0) { //turning
		if (dist != 0) { //turning & driving
			if (liftDown) liftTo(MIDDLE); //lift up so claw doesn't drag on ground
			turn(angle, false, turnConst1, turnConst2, turnConst3); //turn
		} else { //turning but not driving
			turn (angle, true, turnConst1, turnConst2, turnConst3); //turn
			while (abs(turnProgress()) < distCutoff); //wait to throw
		}
	}

	if (dist != 0) { //driving
		driveStraight(dist, true); //drive
		while (driveData.totalDist < distCutoff); //wait to throw
	}

	createLiftManeuver(MAX);
	while (potentiometerVal(lift) < liftThrowPos) maneuvers();
	createClawManeuver(OPEN);
	while (turnData.isTurning || driveData.isDriving || lift.maneuverExecuting || claw.maneuverExecuting) maneuvers();
}

void grabNdump(int delayDuration, int dist=27, int closeTimeout=500) {
	wait1Msec(delayDuration); //wait for objects to be dropped
	closeClaw(closeTimeout);
	turnDriveDump(0, -dist); //dump pillow
}

void driveToWall(int distance=fenceToWallDist) {
	liftTo(BOTTOM);
	driveStraight(distance);
}

void ramToRealign(int duration=500) {
	liftTo(BOTTOM);

	setDrivePower(drive, -127, -127); //realign using wall
	wait1Msec(duration);
}

void initialPillow() {
	setPower(lift, -liftStillSpeed);

	//open claw and drive away from wall
	createClawManeuver(OPEN);
	driveStraight(7, true);
	while(driveData.isDriving) maneuvers();

	//drive to central pillow
	turn(autoSign * -57, true);
	while(turnData.isTurning || claw.maneuverExecuting) maneuvers();
	driveStraight(14);

	closeClaw(); //clamp pillow
}

task skillz() {
	createClawManeuver(OPEN);
	driveStraight(-10, true);
	while (claw.maneuverExecuting || driveData.isDriving) maneuvers();

	grabNdump(2000);

	ramToRealign();

	for (int i=0; i<2; i++) { //dump preload pillows
		driveStraight(fenceToWallDist);

		grabNdump(500);

		ramToRealign();
	}

	//get and dump pillow in center of field
	createLiftManeuver(BOTTOM);
	turn(-33, true, 40, 127, -10);
	while (lift.maneuverExecuting || turnData.isTurning) maneuvers();
	driveStraight(20);
	closeClaw(); //grab pillow
	turnDriveDump(49, -13, 6);

	//grab and dump jacks
	driveStraight(25, true);
	createClawManeuver(HYPEREXTENDED);
	createLiftManeuver(BOTTOM);
	while (driveData.isDriving || claw.maneuverExecuting || lift.maneuverExecuting) maneuvers();
	grabNdump(0, 30, 750);

	//get and dump pillow
	createLiftManeuver(BOTTOM);
	turn(-18, true);
	while (lift.maneuverExecuting || turnData.isTurning) maneuvers();
	driveStraight(30);
	closeClaw();
	turnDriveDump(47, -30, 10);

	//pick up and dump central jacks
	createLiftManeuver(BOTTOM);
	turn(70, true);
	while (turnData.isTurning || lift.maneuverExecuting) maneuvers();
	driveStraight(20, true);
	while (driveData.isDriving) maneuvers();
	closeClaw();
	turnDriveDump(-60, 0);

	liftTo(BOTTOM);
}

task pillowAuton() {
	initialPillow();

	//go to fence and lift up
	createLiftManeuver(TOP);
	driveStraight(10.5, true);
	while (driveData.isDriving) maneuvers();
	turn(autoSign * 57, true, 40, 80, -10); //turn to face fence
	while (turnData.isTurning) maneuvers();
	driveStraight(20, true); // drive up to wall
	while (driveData.isDriving || lift.maneuverExecuting) maneuvers();

	openClaw(); //release pillow
	wait1Msec(600); //wait for pillow to fall
	closeClaw();
	driveStraight(-4); //back up
	hyperExtendClaw();

	//push jacks over
 	driveStraight(6);
 	closeClaw();

 	createClawManeuver(HYPEREXTENDED);

 	//drive to other wall and lift down
 	driveStraight(-10, true);
 	while (driveData.isDriving) maneuvers();
 	turn(autoSign * 65, true, 40, 127, -20);
 	while (turnData.isTurning || claw.maneuverExecuting) maneuvers();
 	createManeuver(lift, liftMiddle+100, liftStillSpeed);
 	driveStraight(35, true);
 	while (driveData.isDriving || lift.maneuverExecuting) maneuvers();
 	turn(autoSign * -65, false, 40, 120, -40);
 	driveStraight(10);

 	goToPosition(lift, liftTop+75); //push jacks over
 	driveStraight(5);
 	closeClaw();
}

task dumpyAuton() {
	initialPillow();

	liftTo(MIDDLE);

	driveStraight(7);
	wait1Msec(500);

	turnDriveDump(autoSign * (-90, -17, 7, 45, 120, -20);
	wait1Msec(250);

	driveToWall();
	grabNdump(0, 30, 750);
	driveToWall();
	grabNdump(0, 30, 750);

	liftTo(BOTTOM);
}

task oneSideAuton() {
	createClawManeuver(HYPEREXTENDED); //open claw
	createManeuver(lift, liftTop-450, liftStillSpeed); //lift to near top
	driveStraight(5, true); //drive away from wall
	while(driveData.isDriving) maneuvers();

	turn(-30, true);
	while (turnData.isTurning) maneuvers();

	driveStraight(18, true);
	while (driveData.isDriving || claw.maneuverExecuting) maneuvers();

	turn(37, true); //turn toward wall
	while (turnData.isTurning || lift.maneuverExecuting) maneuvers();

	//knock off jacks
	driveStraight(42);
	goToPosition(lift, liftTop+250, liftStillSpeed);
	closeClaw();
	wait1Msec(2500);

	//pick up and dump back jacks
	createClawManeuver(OPEN);
	turn(120, true, 40, 127, -10);
	while (turnData.isTurning);
	createLiftManeuver(BOTTOM);
	driveStraight(46, true);
	while (driveData.isDriving || lift.maneuverExecuting || claw.maneuverExecuting) maneuvers();
	closeClaw();
	turnDriveDump(50, -30, 10);

	liftTo(BOTTOM);
}

task autonomous() {
	lift.maneuverExecuting = false;
	claw.maneuverExecuting = false;

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
