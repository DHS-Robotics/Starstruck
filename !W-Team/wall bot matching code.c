#pragma config(Sensor, in1,    pot1,           sensorPotentiometer)
#pragma config(Sensor, in2,    pot2,           sensorPotentiometer)
#pragma config(Sensor, dgtl2,  lock,           sensorDigitalOut)
#pragma config(Motor,  port1,           rd1,           tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           rd2,           tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           lift1,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           lift2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port5,           ld4,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           ld3,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           rd4,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           rd3,           tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           ld2,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          ld1,           tmotorVex393_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

int minus = 0;
int power = 127;

#include "..\Includes\motorGroup.c"

#define liftDiff 900	//difference in lift pot values when at the same height (left - right)

motorGroup leftLift;
motorGroup rightLift;

void pre_auton() {
	initializeGroup(leftLift, 1, lift2);
	addSensor(leftLift, pot2);
	setTargetingPIDconsts(leftLift, 0.2, 0.001, 0.2, 25);

	initializeGroup(rightLift, 1, lift1);
	addSensor(rightLift, pot1);
	setTargetingPIDconsts(rightLift, 0.2, 0.001, 0.2, 25);
}

void setLiftPower(int power) {
	setPower(leftLift, power);
	setPower(rightLift, power);
}

void setLiftTargets(int rightPosition) {
	setTargetPosition(rightLift, rightPosition);
	setTargetPosition(leftLift, rightPosition+liftDiff);
}

void maintainLiftTargets() {
	maintainTargetPos(rightLift);
	maintainTargetPos(leftLift);
}

void usercontrol()
{
	pre_auton();
	while (1==1) {


//drive
		motor[ld1]  = (vexRT[Ch1] + vexRT[Ch2])/2;
		motor[ld2]  = (vexRT[Ch1] + vexRT[Ch2])/2;
		motor[rd1]  = (vexRT[Ch1] - vexRT[Ch2])/2;
		motor[rd2]  = (vexRT[Ch1] - vexRT[Ch2])/2;
	  motor[ld3] = (vexRT[Ch4] - vexRT[Ch3])/2;
	  motor[ld3] = (vexRT[Ch4] - vexRT[Ch3])/2;
	  motor[rd3] = (vexRT[Ch4] + vexRT[Ch3])/2;
	  motor[rd4] = (vexRT[Ch4] + vexRT[Ch3])/2;

//lift
		if (vexRT[Btn5U] == 1) {
			setLiftPower(127);

		} else if (vexRT[Btn5D] == 1) {
			setLiftPower(-127);
		} else {
			//maintainLiftTargets();
			setLiftPower(15);
		}


	}
}
task main() {
	usercontrol();
}
