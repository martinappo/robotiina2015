#include "BallPosition.h"


BallPosition::~BallPosition()
{
}

void BallPosition::setIsUpdated(bool updated) {
	isUpdated = updated;
}

void BallPosition::updateFieldPixelCoords(int x, int y) {
	//TODO: get value relative to field not to robot
	int fieldY = polarPixelCoords.x * cos(TAU*getAngle() / 360);
	int fieldX = polarPixelCoords.x * sin(TAU*getAngle() / 360);

	cv::Point filteredCoords = cv::Point(fieldX, fieldY);//filter->doFiltering(cv::Point(fieldX, fieldY));
	pixelCoordsForField = { 240 + filteredCoords.x, 320 - filteredCoords.y };
}

void BallPosition::predictCoordinates() {
	updateCoordinates(0, 0);
	//pixelCoordsForField = filter->getPrediction();
	//TODO: generate other coordinate types from predicted pixelcoords
}
