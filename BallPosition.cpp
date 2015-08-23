#include "BallPosition.h"


BallPosition::~BallPosition()
{
}

void BallPosition::setIsUpdated(bool updated) {
	isUpdated = updated;
}

void BallPosition::predictCoordinates() {
	updateCoordinates(0, 0, cv::Point(0,0));
	//pixelCoordsForField = filter->getPrediction();
	//TODO: generate other coordinate types from predicted pixelcoords
}
