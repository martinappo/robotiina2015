#include "ObjectPosition.h"


ObjectPosition::ObjectPosition(int distance, int angle) {
	this->polarMetricCoords.x = distance;
	this->polarMetricCoords.y = angle;
}

ObjectPosition::ObjectPosition(cv::Point2i polarCoords) {
	this->polarMetricCoords = polarCoords;
}

ObjectPosition::~ObjectPosition() {};

double ObjectPosition::getDistance() {
	return polarMetricCoords.x;
}

void ObjectPosition::setDistance(int distance) {
	this->polarMetricCoords.x = distance;
}

void ObjectPosition::setFrameSize(cv::Size frameSize) {
	this->frameSize = frameSize;
}

double ObjectPosition::getAngle() {
	return polarMetricCoords.y;
}

double ObjectPosition::getAngleToRobot() {
	return getAngle() + robotAngle;
}

void ObjectPosition::updateCoordinates(int x, int y, cv::Point robotFieldCoords, double robotAngle) {
	this->robotAngle = robotAngle;
	lastFieldCoords = fieldCoords;
	this->rawPixelCoords = { x, y };
	updatePolarCoords();
	updateFieldCoords(robotFieldCoords);
}

void ObjectPosition::updateCoordinates(cv::Point point, cv::Point robotFieldCoords, double robotAngle) {
	return updateCoordinates(point.x, point.y, robotFieldCoords, robotAngle);
}

void ObjectPosition::updatePolarCoords() {
	updatePolarCoords(rawPixelCoords);
}

void ObjectPosition::updatePolarCoords(int x, int y) {
	updatePolarCoords(cv::Point(x, y));
}

void ObjectPosition::updatePolarCoords(cv::Point rawCoords) {
	cv::Point centerOfFrame = { frameSize.width / 2, frameSize.height / 2 };
	double distanceInCm = mDistanceCalculator.getDistance(centerOfFrame.x, centerOfFrame.y, rawCoords.x, rawCoords.y);
	double angle = (angleBetween(rawCoords - centerOfFrame, { 0, -frameSize.height/2 }));
	
	this->polarMetricCoords = { distanceInCm, angle};
}


void ObjectPosition::updateFieldCoords(cv::Point robotFieldCoords) {
	cv::Point centerOfFrame = { frameSize.height / 2, frameSize.width / 2 };
	int fieldY = -1 * (int)(getDistance() * cos(TAU*getAngleToRobot() / 360));
	int fieldX = (int)(getDistance() * sin(TAU*getAngleToRobot() / 360));
	cv::Point filteredCoords = cv::Point(fieldX, fieldY);//filter->doFiltering(cv::Point(fieldX, fieldY));
	fieldCoords = { robotFieldCoords.x + filteredCoords.x, robotFieldCoords.y + filteredCoords.y };
	
}

double ObjectPosition::angleBetween(const cv::Point2i &a, const cv::Point2i &b) {

	double alpha = atan2(a.y, a.x) - atan2(b.y, b.x);
	double alphaDeg = alpha * 180. / CV_PI;
	if (alphaDeg < 0) alphaDeg += 360;
	return alphaDeg;
}





