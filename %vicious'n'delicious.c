#pragma config(Sensor, in1,    ClawPot,        sensorPotentiometer)
#pragma config(Sensor, in2,    LeftPot,        sensorPotentiometer)
#pragma config(Sensor, in3,    ModePot,        sensorPotentiometer)
#pragma config(Sensor, in4,    SidePot,        sensorPotentiometer)
#pragma config(Sensor, in5,    Gyro,           sensorGyro)
#pragma config(Sensor, dgtl1,  rightEncoder,   sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  leftEncoder,    sensorQuadEncoder)
#pragma config(Motor,  port1,           lbd,           tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           lfd,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           lift1,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           lift2,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           claw1,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           claw2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           lift3,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           lift4,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           rfd,           tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          rbd,           tmotorVex393_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX2)
#pragma competitionControl(Competition)
#include "Vex_Competition_Includes.c"

#include "buttonTracker.c"
#include "parallelDrive.c"
#include "pd_autoMove.c"
#include "motorGroup.c"

//buttons
#define toggleLiftModeBtn Btn8U
#define openClawBtn Btn6U //claw
#define closeClawBtn Btn6D
#define liftUpBtn Btn5U //lift
#define liftDownBtn Btn5D

//positions
#define liftMiddle 1239
#define liftVert 2740

//constants
#define liftStillSpeed 10
#define clawStillSpeed 15

short clawSign = 1; //Sign of still speed. Positive if closed, negative if open

motorGroup lift;
motorGroup claw;

//autonomous region
void pre_auton() {
	bStopTasksBetweenModes = true;

	initializeDrive(drive);
  setDriveMotors(drive, 4, lfd, lbd, rfd, rbd);
  attachEncoder(drive, leftEncoder, LEFT);
  attachEncoder(drive, rightEncoder, RIGHT, true, 4);
  attachGyro(drive, Gyro);

	initializeGroup(lift, 4, lift1, lift2, lift3, lift4);
  configureButtonInput(lift, liftUpBtn, liftDownBtn, liftStillSpeed);
  addSensor(lift, LeftPot, true);

  initializeGroup(claw, 2, claw1, claw2);
}
void openclaw (int potValue)
 {
   while(SensorValue[ClawPot] < potValue)//pot value needs to be when the claw is open
   {
     motor[claw1] = 127;
     motor[claw2] = 127;
   }
 }
 void closeclaw (int potValue)
 {
   while(SensorValue[ClawPot] > potValue)//pot value needs to be when the claw is closed
   {
     motor[claw1] = -127;
     motor[claw2] = -127;
   }
 }
 void liftUp (int potValue)
 {
   while(SensorValue[LeftPot] < potValue)//pot value needs to be when the lift is the height desired
   {
     motor[lift1]= 127;
     motor[lift2]= 127;
     motor[lift3]= 127;
     motor[lift4]= 127;
   }
 }
 void liftDown (int potValue)
 {
   while(SensorValue[Leftpot] < potValue)//pot value needs to be when the lift is the height desired
   {
     motor[lift1]= -127;
     motor[lift2]= -127;
     motor[lift3]= -127;
     motor[lift4]= -127;
   }
 }
task rightcube()//right tile
{
	liftUp(1500); //deploy the claw
	wait10Msec(10);
	liftdown(600);//deploy the claw
	/*turn(65);//face the cube
	driveStraight(5);//go to cube
	openclaw(5);
	closeclaw(5);//grab the cube
	turn(5);//face the fence
	driveStraight(5);//go to the fence
	liftUp(3000);//get over it
	openclaw(5);//release the kracken
	closeclaw(5);//cage the kracken
	driveStraight(5);//back-up
	driveStraight(5);//knock over the remainder of the stars on fence*/

}
task leftcube()//left tile
{
	liftUp(5); //deploy the claw
	liftDown(5);//deploy the claw
	turn(5);//face the cube
	driveStraight(5);//go to the cube
	openclaw(5);
	closeclaw(5);//grab the cube
	turn(5);//face the fence
	driveStraight(5);//go to the fence
	liftUp(5);//get over it
	openclaw(5);//release the kracken
	closeclaw(5);//cage the kracken
	driveStraight(5);//back-up
	driveStraight(5);//knock over the remainder of the stars on fence
}
task Rnocube()//in the case teammates auto goes for the cube or with E-team
{
	liftUp(5); //deploy the claw
	liftDown(5);//deploy the claw
	turn(5);//turn to get lined up with fence
	driveStraight(5);//go on the angle provided
	turn(5);//face the fence
	driveStraight(5);//ramming speed spock
	liftUp(5);//size up to the fence
	openclaw(5);//intimindate
	liftUp(5);//finish the wall
	driveStraight(5);//backup
	turn(5);//turn to the back jacks
	openclaw(5);//get ready for jacks
	driveStraight(5);//hoard the jacks
	closeclaw(5);//grab jacks
	driveStraight(5);//backup
	turn(5);//face the wall
	driveStraight(5);//backup to the fence
	liftUp(5);//ready the jacks to dump
	openclaw(5);//release jacks


}
task Lnocube()
{
		liftUp(5); //deploy the claw
	liftDown(5);//deploy the claw
	turn(5);//turn to get lined up with fence
	driveStraight(5);//go on the angle provided
	turn(5);//face the fence
	driveStraight(5);//ramming speed spock
	liftUp(5);//size up to the fence
	openclaw(5);//intimindate
	liftUp(5);//finish the wall
	driveStraight(5);//backup
	turn(5);//turn to the back jacks
	openclaw(5);//get ready for jacks
	driveStraight(5);//hoard the jacks
	closeclaw(5);//grab jacks
	driveStraight(5);//backup
	turn(5);//face the wall
	driveStraight(5);//backup to the fence
	liftUp(5);//ready the jacks to dump
	openclaw(5);//release jacks

}
task autonomous() {
 if (SensorValue[sidePot] > 7 && SensorValue[modePot] > 8 ){
		startTask(rightcube);
	}
	else if (SensorValue[sidePot] < 9 && SensorValue[modePot] < 10) {
		startTask(leftcube);
	}
	else if (SensorValue[sidePot] > 1 && SensorValue[modePot] < 5) {
		startTask(Rnocube);
	}
	else {
		startTask(Lnocube);
	}
}
//end autonomous region

//user control region
void clawControl() {
	if (vexRT[closeClawBtn] == 1) {
		setPower(claw, 127);
		clawSign = 1;
	} else if (vexRT[openClawBtn] == 1) {
		setPower(claw, -127);
		clawSign = -1;
	} else {
		setPower(claw, clawStillSpeed * clawSign);
	}
}
void liftControl() {
	short potPos = potentiometerVal(lift);
	lift.stillSpeed = liftStillSpeed * ((potPos<liftMiddle || potPos>liftVert) ? -1 : 1);
	takeInput(lift);
}

task usercontrol() {
	while (true) {
  	driveRuntime(drive);

  	liftControl();

		clawControl();
  }
}
//end user control region
