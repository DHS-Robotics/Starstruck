#pragma config(Sensor, in1,    Yaw,            sensorGyro)
#pragma config(Sensor, dgtl3,  leftE,          sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  rightE,         sensorQuadEncoder)
#pragma config(Motor,  port1,           Seymore,       tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           lfd,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           lbd,           tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           rfd,           tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           rbd,           tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          FeedMe,        tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "parallelDrive.c"
#include "pd_autoMove.c"
#include "motorGroup.c"

motorGroup feed;

float debug[5] = {0, 0, 0, 0, 0};

void autonomous() {
	driveStraight(drive, 24);
	turn(drive, -90);
	driveStraight(drive, 48);
}

task main() {
	initializeDrive(drive);
	setDriveMotors(drive, 4, lfd, lbd, rfd, rbd);
	attachGyro(drive, Yaw);
	attachEncoder(drive, leftE, LEFT);
	attachEncoder(drive, rightE, RIGHT, true);

	initializeGroup(feed, 2, Seymore, FeedMe);
	configureButtonInput(feed, Btn7U, Btn7D);

	while (true) {
		driveRuntime(drive);
		updatePosition(drive);

		takeInput(feed);

		debug[0] = driveEncoderVal(drive);
		debug[1] = driveEncoderVal(drive, LEFT);
		debug[2] = driveEncoderVal(drive, RIGHT);
		debug[3] = SensorValue[leftE];
		debug[4] = SensorValue[rightE];

		if (vexRT[Btn5U] == 1) autonomous();

		if (vexRT[Btn5D] == 1) drive.width = calculateWidth(drive);

		if (vexRT[Btn6D] == 1) {
			setRobotPosition(drive, 0, 0, 0);
			resetGyro(drive);
		}
	}
}
