#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "KalmanFilter.h"
#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class BallPosition : public ObjectPosition
{
public:
	BallPosition(){};
	BallPosition(const BallPosition & ballPos){};
	virtual ~BallPosition();
	int id;
	bool isValid;
	std::atomic_bool isUpdated;
	void predictCoordinates();
	void setIsUpdated(bool updated);
	void updateFieldCoords(cv::Point2d orgin = cv::Point2d(0, 0));
	// for simulator
	double speed = 0;
	double heading = 0;
private:
	KalmanFilter filter = KalmanFilter(cv::Point2i(0, 0));
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
};
