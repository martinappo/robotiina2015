#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class BallPosition : public ObjectPosition
{
public:
	BallPosition(){};
	BallPosition(const BallPosition & ballPos){
		rawPixelCoords = ballPos.rawPixelCoords;
		polarMetricCoords = ballPos.polarMetricCoords;
		fieldCoords = ballPos.fieldCoords;
	};
	virtual ~BallPosition();
	int id;
	bool isValid;
	std::atomic_bool isUpdated;
	void setIsUpdated(bool updated);
	void updateFieldCoords(cv::Point2d orgin = cv::Point2d(0, 0), double heading = 0);
	// for simulator
	double speed = 0;
	double heading = 0;
	void filterCoords(const BallPosition &ball, bool reset=false);
	void predictCoords();
	cv::Point2i lastRawCoords;
private:
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
};
