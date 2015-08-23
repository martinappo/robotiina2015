#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "KalmanFilter.h"

class BallPosition : public ObjectPosition
{
public:
#ifdef WIN32
	BallPosition() {};
#else
	BallPosition() noexcept{};
#endif
	virtual ~BallPosition();
	int id;
	bool isValid;
	bool isUpdated;
	void predictCoordinates();
	void setIsUpdated(bool updated);
private:
	KalmanFilter* filter = new KalmanFilter(cv::Point2i(400, 400));
};
