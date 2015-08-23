#include "GatePosition.h"

GatePosition::GatePosition(int x, int y) {
	this->polarMetricCoords = cv::Point(x, y);
	this->fieldCoords = cv::Point(x, y);
}

GatePosition::GatePosition(OBJECT gate) {
	if (gate == YELLOW_GATE) {
		this->polarMetricCoords = cv::Point(0, 155);
		this->fieldCoords = cv::Point(0, 155);
	}
	else if (gate == BLUE_GATE) {
		this->polarMetricCoords = cv::Point(500, 155);
		this->fieldCoords = cv::Point(500, 155);
	}
	else {
		throw std::runtime_error("Gate must be either BLUE_GATE or YELLOW_GATE");
	}
}

GatePosition::~GatePosition()
{
}

void GatePosition::updateCoordinates(int x, int y, cv::Point dummy) {
	this->rawPixelCoords = { x, y };
	updatePolarCoords();
	//Coordinates relative to field are fixed for gates so we are not updating them
}

void GatePosition::updateCoordinates(cv::Point rawCoords, cv::Point dummy) {
	this->rawPixelCoords = rawCoords;
	updatePolarCoords();
	//Coordinates relative to field are fixed for gates so we are not updating them
}

void GatePosition::updateFieldCoords() {
	//no
	return;
}