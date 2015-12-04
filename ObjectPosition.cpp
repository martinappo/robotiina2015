#include "ObjectPosition.h"
#include "DistanceCalculator.h"
extern DistanceCalculator gDistanceCalculator;



void ObjectPosition::updateRawCoordinates(const cv::Point2d pos, cv::Point2d orgin) {
	lastFieldCoords = fieldCoords;
	rawPixelCoords = pos;
	polarMetricCoords = gDistanceCalculator.getPolarCoordinates(orgin, pos);
}