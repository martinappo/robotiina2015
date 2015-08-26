#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "GatePosition.h"

class RobotPosition : public ObjectPosition
{
public:
#ifdef WIN32
	RobotPosition() {
		this->polarMetricCoords = cv::Point(0, 0);
	};
#else
	RobotPosition() noexcept {
		this->polarMetricCoords = cv::Point(0, 0);
	};
#endif
	RobotPosition(GatePosition yellowGate, GatePosition blueGate, cv::Point initialCoords = cv::Point(0,0));
	virtual ~RobotPosition();
	virtual void updatePolarCoords();
	virtual void updateFieldCoords(GatePosition yellowGate, GatePosition blueGate);
	virtual void updateCoordinates(GatePosition yellowGate, GatePosition blueGate);
private:
	void initPolarCoordinates();
	std::pair<cv::Point, cv::Point> intersectionOfTwoCircles(cv::Point circle1center, double circle1Rad, cv::Point circle2center, double circle2Rad);
};
