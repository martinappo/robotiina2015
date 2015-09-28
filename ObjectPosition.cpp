#include "ObjectPosition.h"
#include "DistanceCalculator.h"
extern DistanceCalculator gDistanceCalculator;



void ObjectPosition::updateRawCoordinates(const cv::Point pos, cv::Point orgin) {
	lastFieldCoords = fieldCoords;
	rawPixelCoords = pos;
	double distanceInCm = gDistanceCalculator.getDistance(orgin, pos);
	double angle = angleBetween(pos - orgin, { 0, -1 });

	polarMetricCoords = { distanceInCm, angle };
//	updateFieldCoords(cv::Point());
}



double ObjectPosition::angleBetween(const cv::Point2i &a, const cv::Point2i &b) {

	double alpha = atan2(a.y, a.x) - atan2(b.y, b.x);
	double alphaDeg = alpha * 180. / CV_PI;
	if (alphaDeg < 0) alphaDeg += 360;
	return alphaDeg;
}





