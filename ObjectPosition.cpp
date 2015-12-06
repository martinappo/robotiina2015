#include "ObjectPosition.h"
#include "DistanceCalculator.h"
extern DistanceCalculator gDistanceCalculator;

void ObjectPosition::updateRawCoordinates(const cv::Point2d pos, cv::Point2d orgin, bool useKalman) {
	lastFieldCoords = fieldCoords;
	if (useKalman) {
		rawPixelCoords = filter.doFiltering(pos);
	}
	else {
		rawPixelCoords = pos;
	}
	
	polarMetricCoords = gDistanceCalculator.getPolarCoordinates(orgin, pos);
}