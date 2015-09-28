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

void BallPosition::updateFieldCoords(cv::Point orgin) {

	int fieldY = -(int)(polarMetricCoords.x * cos(TAU*polarMetricCoords.y / 360));
	int fieldX = (int)(polarMetricCoords.x * sin(TAU*polarMetricCoords.y / 360));
	cv::Point filteredCoords = true ? cv::Point(fieldX, fieldY) : filter.doFiltering(cv::Point(fieldX, fieldY));
	fieldCoords = orgin + filteredCoords;

}
