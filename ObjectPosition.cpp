#include "ObjectPosition.h"

ObjectPosition::ObjectPosition(int distance, int angle) {
	this->polarMetricCoords.x = distance;
	this->polarMetricCoords.y = angle;
}

ObjectPosition::ObjectPosition(cv::Point2i polarCoords) {
	this->polarMetricCoords = polarCoords;
}

//ObjectPosition::ObjectPosition() {
//
//}

ObjectPosition::~ObjectPosition() {};

int ObjectPosition::getDistance() {
	return polarMetricCoords.x;
}

void ObjectPosition::setDistance(int distance) {
	this->polarMetricCoords.x = distance;
}

int ObjectPosition::getAngle() {
	return polarMetricCoords.y;
}

double ObjectPosition::angleBetween(const cv::Point2i &a, const cv::Point2i &b, const cv::Point2i &c) {
	cv::Point2i ab = { b.x - a.x, b.y - a.y };
	cv::Point2i cb = { b.x - c.x, b.y - c.y };

	double dot = (ab.x * cb.x + ab.y * cb.y); // dot product
	double cross = (ab.x * cb.y - ab.y * cb.x); // cross product

	double alpha = atan2(cross, dot);
	double alphaDeg = floor(alpha * 180. / CV_PI + 0.5);
	if (alphaDeg < 0) {
		return 360 + alphaDeg;
	}
	return alphaDeg;
}


void ObjectPosition::updateCoordinates(int x, int y) {
	this->rawPixelCoords = { x, y };
	updatePolarCoords(x, y);
	updateFieldPixelCoords(x, y);
	updateMetricCoords(x, y);
}

void ObjectPosition::updateCoordinates(cv::Point point) {
	return updateCoordinates(point.x, point.y);
}

void ObjectPosition::updatePolarCoords(int x, int y) {
	cv::Point centerOfFrame = { frameSize.height / 2, frameSize.width / 2 };
	int distanceInPixels = cv::norm(rawPixelCoords - centerOfFrame);
	//TODO: get value from lookuptable not randomly
	int distanceInCm = 500 * distanceInPixels / (frameSize.height / 2 + 1);
	int angle = angleBetween(rawPixelCoords, centerOfFrame, { frameSize.height, frameSize.width / 2 });
	this->polarMetricCoords = { distanceInCm, angle };
	this->polarPixelCoords = { distanceInPixels, angle };
}

void ObjectPosition::updateMetricCoords(int x, int y) {
	//TODO: get value relative to field not to robot
	int metricX = 500 * pixelCoordsForField.x / (frameSize.height / 2 + 1);
	int metricY = 500 * pixelCoordsForField.y / (frameSize.height / 2 + 1);
	this->metricCoords = { metricX, metricY };
}

void ObjectPosition::updateFieldPixelCoords(int x, int y) {
	//TODO: get value relative to field not to robot
	int fieldY = polarPixelCoords.x * cos(TAU*getAngle() / 180);
	int fieldX = polarPixelCoords.x * sin(TAU*getAngle() / 180);
	this->pixelCoordsForField = { 320 + fieldX, 240 + fieldY };
}





