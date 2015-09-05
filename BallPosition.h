#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "KalmanFilter.h"

class BallPosition : public ObjectPosition
{
public:
	BallPosition() {};
	virtual ~BallPosition();
	int id;
	bool isValid;
	std::atomic_bool isUpdated;
	void predictCoordinates();
	void setIsUpdated(bool updated);
private:
	KalmanFilter* filter = new KalmanFilter(cv::Point2i(400, 400));
};
