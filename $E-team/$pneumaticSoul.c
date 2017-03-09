#pragma config(Sensor, in1,    hyro,           sensorGyro)
#pragma config(Sensor, in2,    clawPotR,       sensorPotentiometer)
#pragma config(Sensor, in3,    liftPot,        sensorPotentiometer)
#pragma config(Sensor, in4,    modePot,        sensorPotentiometer)
#pragma config(Sensor, in5,    sidePot,        sensorPotentiometer)
#pragma config(Sensor, in6,    clawPotL,       sensorPotentiometer)
#pragma config(Sensor, dgtl1,  leftEncoder,    sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  rightEncoder,   sensorQuadEncoder)
#pragma config(Motor,  port1,           RB,            tmotorVex393HighSpeed_HBridge, openLoop)
#pragma config(Motor,  port2,           L2,            tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port3,           Lt1,           tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port4,           Lt2,           tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port5,           CR,            tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port6,           CL,            tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           Lt3,           tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           Lt4,           tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           R2,            tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          WB,            tmotorVex393HighSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//#region mode config
//#define DRIVER_PID	//uncommented if using PID instead of still speeds during user control
//#endregion

//#region setup
#pragma platform(VEX2)
#pragma competitionControl(Competition)
#include "Vex_Competition_Includes.c"
#include "..\Includes\pd_autoMove.c"
//#endregion

//#region buttons
	//#subregion claw
#define autoDumpOnBtn			Btn8U
#define autoDumpOffBtn		Btn8D
#define clawForwardBtn		Btn7R	//directly sets claw power to clawDefPower
#define clawBackwardBtn		Btn7L	//directly sets claw power to -clawDefPower
#define clawNeutralBtn		Btn7U	//directly sets claw power to 0
#define openClawBtn				Btn6U
#define closeClawBtn			Btn6D
#define hyperExtendBtn		Btn7D
	//#endsubregion
	//#subregion lift
#define liftUpBtn		Btn5U
#define liftDownBtn	Btn5D
	//#endsubregion
//#endregion

//#region enums
enum liftState { BOTTOM, MIDDLE, TOP, THROW, MAX };
enum clawState { CLOSED, OPEN, STRAIGHT, HYPEREXTENDED, NARUTO };
//#endregion

//#region positions
int liftPositions[5] = { 795, 1557, 2000, 2000, 2400 };	//same order as corresponding enums
int clawPositions[5] = { 400, 1250, 1550, 3000, 4000 };
//#endregion

//#region constants
#define liftStillSpeed 15
#define clawDefPower 80	//power used in manual control
#define liftErrorMargin 150	//margins of error
#define clawErrorMargin 100
#define maxStationarySpeed	100	//max error decrease in claw PID error (per second) where claw is considered not to be moving (CURRENTLY UNUSED)
#define fenceToWallDist 30
#define clawDiff 0					//difference between claw potentiometers when at the same angle (left - right)
#define liftDriftDist	300	//estimated distance lift drifts after button is released (for setting lift PID target during drive control)
//#endregion

//#region auton config
#define dumpToSide false
#define straightToCube true
#define blocking false
#define agressiveClose false
//#endregion

//#region timers
#define autonTimer T1
#define movementTimer T2
//#region

//#region globals
bool autoDumping = true;
int autoSign; //for autonomous, positive if robot is left of pillow
clawState currentState;
int liftDirection;

motorGroup lift;
motorGroup rightClaw;
motorGroup leftClaw;
//#endregion

void pre_auton() {
	bStopTasksBetweenModes = true;

	initializeAutoMovement();

	turnDefaults.rampConst1 = 40;
	turnDefaults.rampConst2 = 127;
	turnDefaults.rampConst3 = -30;

	driveDefaults.rampConst1 = 50;
	driveDefaults.rampConst2 = 120;
	driveDefaults.rampConst3 = -20;

	//configure drive and lift motors
	initializeDrive(drive, true);
	setDriveMotors(drive, 2, L2, R2);
	initializeGroup(lift, 4, Lt1, Lt2, Lt3, Lt4);

	//finish configuring drive
	attachEncoder(drive, leftEncoder, LEFT);
	attachEncoder(drive, rightEncoder, RIGHT, false, 3.25);
	attachGyro(drive, hyro);

	//finish configuring lift
	configureButtonInput(lift, liftUpBtn, liftDownBtn, liftStillSpeed);
	addSensor(lift, liftPot);

	//configure claw sides
	initializeGroup(rightClaw, 1, CR);
	setTargetingPIDconsts(rightClaw, 0.2, 0, 0.7/25, 25);
	addSensor(rightClaw, clawPotR);

	initializeGroup(leftClaw, 1, CL);
	setTargetingPIDconsts(leftClaw, 0.2, 0, 0.7/25, 25);
	addSensor(leftClaw, clawPotL);
}

void inactivateTargets() {
	stopTargeting(lift);
	stopTargeting(rightClaw);
	stopTargeting(leftClaw);
}

//#region lift
void setLiftState(liftState state) {
	setTargetPosition(lift, liftPositions[state]);
}

void setLiftPIDmode(bool auto) {
	if (auto)
		setTargetingPIDconsts(lift, 0.4, 0.001, 0.6, 25);
	else
		setTargetingPIDconsts(lift, 0.2, 0.001, 0.2, 25);
}

void liftControl() {
	#ifdef DRIVER_PID
		if (vexRT[liftUpBtn] == 1) {
			setPower(lift, 127);
			setTargetPosition(lift, limit(getPosition(lift)+liftDriftDist, liftPositions[BOTTOM], liftPositions[MAX]));
		} else if (vexRT[liftDownBtn] == 1) {
			setPower(lift, -127);
			setTargetPosition(lift, limit(getPosition(lift)-liftDriftDist, liftPositions[BOTTOM], liftPositions[MAX]));
		} else {
			maintainTargetPos(lift);
		}
	#else
		if (vexRT[liftUpBtn] == 1) {
			setPower(lift, 127);
			liftDirection = 1;
		} else if (vexRT[liftDownBtn] == 1) {
			setPower(lift, -127);
			liftDirection = -1;
		} else {
			setPower(lift, liftDirection * liftStillSpeed);
		}
	#endif
}
//#endregion

//#region claw
void executeClawPIDs() {
	maintainTargetPos(leftClaw);
	maintainTargetPos(rightClaw);
}

void setClawState(clawState state) {
	setTargetPosition(leftClaw, clawPositions[state]+clawDiff);
	setTargetPosition(rightClaw, clawPositions[state]);
	currentState = state;
}

void setClawTargets(int targetPos) {
	setTargetPosition(leftClaw, targetPos+clawDiff);
	setTargetPosition(rightClaw, targetPos);
}

void setClawPower(int power, bool setTargets=true) {
	setPower(leftClaw, power);
	setPower(rightClaw, power);

	if (setTargets) setClawTargets(getPosition(rightClaw));
}

void clawControl() {
	if (vexRT[clawNeutralBtn] == 1) {
		setClawPower(0);
	} else if (vexRT[clawForwardBtn] == 1) {
		setClawPower(clawDefPower);
	} else	if (vexRT[clawBackwardBtn] == 1) {
		setClawPower(-clawDefPower);
	} else if (vexRT[openClawBtn]==1 && currentState!=OPEN)
		setClawState(OPEN);
	else if (vexRT[closeClawBtn]==1 && currentState!=CLOSED)
		setClawState(CLOSED);
	else if (vexRT[hyperExtendBtn]==1 && currentState!=HYPEREXTENDED)
		setClawState(HYPEREXTENDED);
	else if (getPosition(lift)>liftPositions[THROW] && currentState!=OPEN && autoDumping)
		setClawState(OPEN);
	else {
		executeClawPIDs();
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
		executeClawPIDs();

		maintainTargetPos(lift);

		EndTimeSlice();
	}
}

bool clawNotMoving(motorGroup *claw, int maxSpeed=maxStationarySpeed) {
	float speed = (abs(claw->posPID.target - getPosition(claw)) - abs(claw->posPID.prevError)) * 1000 / time(claw->posPID.lastUpdated);
	return speed < maxSpeed;
}

bool clawIsClosed(int maxSpeed=maxStationarySpeed) {
	return getPosition(rightClaw) < clawPositions[OPEN] /*&& clawNotMoving(rightClaw, maxSpeed)*/
					&& getPosition(leftClaw) < clawPositions[OPEN]+clawDiff /*&& clawNotMoving(leftClaw, maxSpeed)*/;
}

bool liftNotReady() { return !errorLessThan(lift, liftErrorMargin); }

bool clawNotReady() {
	if (rightClaw.posPID.target == clawPositions[CLOSED])	//assuming this is representative of both claw sides
		return !clawIsClosed();
	else
		return !(errorLessThan(rightClaw, clawErrorMargin) || errorLessThan(leftClaw, clawErrorMargin));
}

void waitForMovementToFinish(bool waitForClaw=true, bool waitForLift=true, bool waitForDrive=true, int timeout=75) {
	clearTimer(movementTimer);

	while (time1(movementTimer) < timeout) {
		if ((liftNotReady() && waitForLift) || (clawNotReady() && waitForClaw))
			clearTimer(movementTimer);
		EndTimeSlice();
	}

	while (turnData.isTurning || driveData.isDriving) EndTimeSlice();
}

void liftTo(liftState state, int timeout=75) {
	setLiftState(state);
	clearTimer(movementTimer);

	while (time1(movementTimer) < timeout) {
		if (liftNotReady())
			clearTimer(movementTimer);
		EndTimeSlice();
	}
}

void moveClawTo(clawState state, int timeout=75) {
	setClawState(state);
	clearTimer(movementTimer);

	while (time1(movementTimer) < timeout) {
		if (clawNotReady())
			clearTimer(movementTimer);
		EndTimeSlice();
	}
}

void turnDriveDump (int angle, int dist, int distCutoff=0, float turnConst1=turnDefaults.rampConst1, float turnConst2=turnDefaults.rampConst2, float turnConst3=turnDefaults.rampConst3) {
	if (angle != 0) { //turning
		if (dist != 0) { //turning & driving
			if (lift.posPID.target < liftPositions[MIDDLE])
				liftTo(MIDDLE); //lift up so claw doesn't drag on ground
			turn(angle, false, turnConst1, turnConst2, turnConst3); //turn
		} else { //turning but not driving
			turn(angle, true, turnConst1, turnConst2, turnConst3); //turn
			while (abs(turnProgress()) < distCutoff) EndTimeSlice(); //wait to throw
		}
	}

	if (dist != 0) { //driving
		driveStraight(dist, true); //drive
		while (driveData.totalDist<distCutoff && driveData.isDriving) EndTimeSlice(); //wait to throw
	}

	setLiftState(MAX);
	while (getPosition(lift) < liftPositions[THROW]) EndTimeSlice();
	setClawState(OPEN);
	waitForMovementToFinish();
}

void grabNdump(int delayDuration, int dist=fenceToWallDist, int closeTimeout=500) {
	wait1Msec(delayDuration); //wait for objects to be dropped
	moveClawTo(CLOSED, closeTimeout);
	turnDriveDump(0, -dist); //dump pillow
}

void ramToRealign(int duration=500, bool liftToBottom=true) {
	if (liftToBottom) liftTo(BOTTOM);

	setDrivePower(drive, -127, -127); //realign using wall
	wait1Msec(duration);
	setDrivePower(drive, 0, 0);
}

task skillz() {
	driveStraight(-13);
	setClawState(OPEN);
	liftTo(MIDDLE);
	liftTo(BOTTOM);

	grabNdump(3000);

	for (int i=0; i<2; i++) { //dump preload pillows
		ramToRealign();

		driveStraight(fenceToWallDist);

		grabNdump(500);
	}

	ramToRealign();

	//get and dump front center jacks
	setTargetPosition(lift, liftPositions[MIDDLE]+100);
	setClawTargets(clawPositions[OPEN]-100);
	ramToRealign(500, false);
	driveStraight(6, true);
	waitForMovementToFinish(false);
	turn(-55, true, 40, 90, -30);
	waitForMovementToFinish();
	liftTo(BOTTOM);
	driveStraight(40);
	moveClawTo(CLOSED);
	wait1Msec(500);
	setTargetPosition(lift, liftPositions[MIDDLE]+250);
	driveStraight(-6);
	turnDriveDump(68, -5, 0, 40, 95, -30);

	//get and dump pillow in center of field
	setLiftState(MIDDLE);
	ramToRealign(500, false);
	setLiftState(BOTTOM);
	waitForMovementToFinish();
	driveStraight(13);
	moveClawTo(CLOSED); //grab pillow
	turnDriveDump(0, -15);

	//grab and dump center back jacks
	setClawState(HYPEREXTENDED);
	ramToRealign();
	driveStraight(fenceToWallDist+2, true);
	waitForMovementToFinish();
	grabNdump(0, fenceToWallDist+10, 750);

	//get and dump right side pillow
	ramToRealign();
	driveStraight(3);
	turn(-17, true);
	waitForMovementToFinish();
	driveStraight(50);
	moveClawTo(CLOSED);
	turnDriveDump(22, -fenceToWallDist, 10);

	//for redundancy
	ramToRealign();
	//driveStraight(fenceToWallDist);
	//grabNdump(500);

	//get first right side jack
	setLiftState(MIDDLE);
	driveStraight(5, true);
	waitForMovementToFinish();
	turn(-55);
	liftTo(BOTTOM);
	driveStraight(10);

	//get second side jack
	turn(70);
	driveStraight(fenceToWallDist);
	grabNdump(0);

	//for redundancy
	for (int i=0; i>=0; i+=5) {
		ramToRealign();
		driveStraight(fenceToWallDist + i);
		grabNdump(500);
	}
}

task autonomous() {
	inactivateTargets();
	setLiftPIDmode(true);
	startTask(maneuvers);

	startTask(skillz);

	while (true) EndTimeSlice();
}
//#endregion

task usercontrol() {
	inactivateTargets();
	setClawPower(0);
	setPower(lift, 0);
	setLiftPIDmode(false);

	while (true) {
		driveRuntime(drive);

		motor[RB] = 127 * (vexRT[Btn7L] - vexRT[Btn7R]);
		motor[WB] = 127 * (vexRT[Btn7U] - vexRT[Btn7D]);

		liftControl();

		clawControl();
	}
}
