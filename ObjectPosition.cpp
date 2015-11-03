#include "ObjectPosition.h"
#include "DistanceCalculator.h"
extern DistanceCalculator gDistanceCalculator;



void ObjectPosition::updateRawCoordinates(const cv::Point2d pos, cv::Point2d orgin) {
	lastFieldCoords = fieldCoords;
	rawPixelCoords = pos;
	double distanceInCm = gDistanceCalculator.getDistance(orgin, pos);
	double angle = DistanceCalculator::angleBetween(pos - orgin, { 0, -1 });

	polarMetricCoords = { distanceInCm, angle };
//	updateFieldCoords(cv::Point());
}








