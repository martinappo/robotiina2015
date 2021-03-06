#include "BallPosition.h"
#include "DistanceCalculator.h"
extern DistanceCalculator gDistanceCalculator;

BallPosition::~BallPosition()
{
}

void BallPosition::setIsUpdated(bool updated) {
	isUpdated = updated;
}
void BallPosition::filterCoords(const BallPosition &ball, bool reset) {
	if (reset) {
		filter.reset(ball.rawPixelCoords);
		filteredRawCoords = ball.rawPixelCoords;
	}
	else {
		filteredRawCoords = filter.doFiltering(ball.rawPixelCoords);
	}
}

void BallPosition::predictCoords() {
	filteredRawCoords = filter.getPrediction();
}

void BallPosition::updateFieldCoords(cv::Point2d orgin, double heading) {
	double fieldY = -(polarMetricCoords.x * cos(TAU*(heading+polarMetricCoords.y) / 360));
	double fieldX = (polarMetricCoords.x * sin(TAU*(heading+polarMetricCoords.y) / 360));
	cv::Point2d filteredCoords = true ? cv::Point2d(fieldX, fieldY) : filter.doFiltering(cv::Point2d(fieldX, fieldY));
	fieldCoords = orgin + filteredCoords;
	time = boost::posix_time::microsec_clock::local_time();

	double dt = (double)(time - lastStep).total_milliseconds() / 1000.0;
	if (lastFieldCoords.x < 10000) { // calculate speed
		//v = 
	}
	lastFieldCoords = fieldCoords;
	lastStep = time;

}
