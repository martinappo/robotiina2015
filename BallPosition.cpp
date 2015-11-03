#include "BallPosition.h"


BallPosition::~BallPosition()
{
}

void BallPosition::setIsUpdated(bool updated) {
	isUpdated = updated;
}

void BallPosition::predictCoordinates() {
	//updateCoordinates(0, 0, cv::Point(0,0), 0);
	//pixelCoordsForField = filter->getPrediction();
	//TODO: generate other coordinate types from predicted pixelcoords
}

void BallPosition::updateFieldCoords(cv::Point2d orgin) {

	double fieldY = -(polarMetricCoords.x * cos(TAU*polarMetricCoords.y / 360));
	double fieldX = (polarMetricCoords.x * sin(TAU*polarMetricCoords.y / 360));
	cv::Point2d filteredCoords = true ? cv::Point2d(fieldX, fieldY) : filter.doFiltering(cv::Point2d(fieldX, fieldY));
	fieldCoords = orgin + filteredCoords;

}
