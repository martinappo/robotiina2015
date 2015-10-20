#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "KalmanFilter.h"

class BallPosition : public ObjectPosition
{
public:
	BallPosition(){};
	virtual ~BallPosition();
	int id;
	bool isValid;
	std::atomic_bool isUpdated;
	void predictCoordinates();
	void setIsUpdated(bool updated);
	void updateFieldCoords(cv::Point orgin = cv::Point(0, 0));
	// for simulator
	double speed = 0;
	double heading = 0;

private:
	KalmanFilter filter = KalmanFilter(cv::Point2i(0, 0));
};
