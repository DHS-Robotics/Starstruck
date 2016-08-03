#pragma config(Sensor, dgtl1,  pneumaticSoul,  sensorDigitalOut)
#pragma config(Motor,  port1,           rbd,           tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           lbd,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           lfd,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           rfd,           tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port5,           lift1,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           lift2,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           lift3,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           lift4,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           claw1,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          claw2,         tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX2)
#pragma competitionControl(Competition)
#include "Vex_Competition_Includes.c"

#include "buttonTracker.c"
#include "parallelDrive.c"

#define liftUpBtn Btn5U
#define liftDownBtn Btn5D
#define clawUpBtn Btn6U
#define clawDownBtn Btn6D
#define toggleClawBtn Btn7U

parallel_drive drive;

//set functions region
void setLiftPower(int power) {
  motor[lift1] = power;
  motor[lift2] =  power;
  motor[lift3] = power;
  motor[lift4] = power;
}

void setClawPower(int power) {
  motor[claw1] = power;
  motor[claw2] = power;
}
//end set funcitons region

void pre_auton() { bStopTasksBetweenModes = true; }

task autonomous() {
  AutonomousCodePlaceholderForTesting();
}

task usercontrol() {
  initializeDrive(drive);
  setLeftMotors(drive, 2, lfd, lbd);
  setRightMotors(drive, 2, rfd, rbd);

  while (true) {
    driveRuntime(drive);

    setLiftPower(127*vexRT[liftUpBtn] - 127*vexRT[liftDownBtn]);
    setClawPower(127*vexRT[clawUpBtn] - 127*vexRT[clawDownBtn]);

    if (newlyPressed(toggleClawBtn)) SensorValue[pneumaticSoul] = 1 - SensorValue[pneumaticSoul];
  }
}
