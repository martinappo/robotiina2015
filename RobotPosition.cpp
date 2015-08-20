#include "RobotPosition.h"

RobotPosition::RobotPosition() {
	this->polarMetricCoords = cv::Point(0, 0);
}

RobotPosition::RobotPosition(GatePosition yellowGate, GatePosition blueGate) {
	this->polarMetricCoords = cv::Point(0, 0);
	updateCoordinates(yellowGate, blueGate);
}

RobotPosition::RobotPosition(int x, int y) {
	this->polarMetricCoords = cv::Point(0, 0);
	this->polarMetricCoords = cv::Point(x, y);
	this->fieldCoords = cv::Point(x, y);
}

RobotPosition::~RobotPosition()
{
}

void RobotPosition::updateCoordinates(GatePosition yellowGate, GatePosition blueGate) {
	updateFieldCoords(yellowGate, blueGate);
}

void RobotPosition::updateFieldCoords(GatePosition yellowGate, GatePosition blueGate) {
	this->fieldCoords = cv::Point(250, 155);
}

void RobotPosition::updatePolarCoords() {
	//Polar coords are fixed to self
	return;
}
