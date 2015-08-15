#include "ObjectPosition.h"

ObjectPosition::ObjectPosition(int distance, int angle) {
	this->polarCoords.x = distance;
	this->polarCoords.y = angle;
}

ObjectPosition::ObjectPosition(cv::Point2i polarCoords) {
	this->polarCoords = polarCoords;
}

ObjectPosition::ObjectPosition() {
	updateCoordinates(0, 0);
}

ObjectPosition::~ObjectPosition() {};

int ObjectPosition::getDistance() {
	return polarCoords.x;
}

void ObjectPosition::setDistance(int distance) {
	this->polarCoords.x = distance;
}

int ObjectPosition::getAngle() {
	return polarCoords.y;
}

double ObjectPosition::angleBetween(const cv::Point2i &v1, const cv::Point2i &v2) {
	return atan2(v2.y - v1.y, v2.x - v1.x) * 180.0 / CV_PI;
}

void ObjectPosition::updateCoordinates(int x, int y) {
	rawPixelCoords = { x, y };
	updatePolarCoords(x, y);
	updateFieldPixelCoords(x, y);
	updateMetricCoords(x, y);
}

void ObjectPosition::updateCoordinates(cv::Point point) {
	return updateCoordinates(point.x, point.y);
}

void ObjectPosition::updatePolarCoords(int x, int y) {
	double distanceInPixels = cv::norm(center - rawPixelCoords);
	//TODO: get value from lookuptable not randomly
	int distanceInCm = 5 * distanceInPixels / (frameSize.height / 2);
	int angle = angleBetween(rawPixelCoords, center);
	polarCoords = { distanceInCm, angle };
}

void ObjectPosition::updateMetricCoords(int x, int y) {
	//TODO: get value relative to field not to robot
	int metricX = 5 * pixelCoordsForField.x / (frameSize.height / 2);
	int metricY = 5 * pixelCoordsForField.y / (frameSize.height / 2);
	metricCoords = { metricX, metricY };
}

void ObjectPosition::updateFieldPixelCoords(int x, int y) {
	//TODO: get value relative to field not to robot
	int fieldY = getDistance() * cos(TAU*getAngle() / 360) / 16;
	int fieldX = getDistance() * sin(TAU*getAngle() / 360) / 16;
	pixelCoordsForField = { 320 + fieldX, 240 - fieldY };
}





