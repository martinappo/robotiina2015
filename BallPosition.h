#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "KalmanFilter.h"

class BallPosition : public ObjectPosition
{
public:
	BallPosition();
	virtual ~BallPosition();
	int id;
	bool isValid;
	bool isUpdated;
	void predictCoordinates();
	virtual void updateFieldPixelCoords(int x, int y);
	void BallPosition::setIsUpdated(bool updated);
private:
	KalmanFilter* filter = new KalmanFilter(cv::Point2i(400, 400));
};