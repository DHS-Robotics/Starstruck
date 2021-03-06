#pragma config(Motor,  port1,           ld,            tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           feedMe,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           seymour,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          rd,            tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

void setFeedPower(int power) {
	motor[feedMe] = power;
	motor[seymour] = power;
}

void setDrivePower(int left, int right) {
	motor[ld] = left;
	motor[rd] = right;
}

task main() {
	while (true) {
		setDrivePower(vexRT[Ch3], vexRT[Ch2]);

		setFeedPower(127*vexRT[Btn6U] - 127*vexRT[Btn6D]);
	}
}
