#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "GatePosition.h"

class RobotPosition : public ObjectPosition
{
public:
	RobotPosition(GatePosition &yellowGate, GatePosition &blueGate, cv::Point initialCoords = cv::Point(0,0));
	virtual ~RobotPosition();
	virtual void updatePolarCoords();
	virtual void updateFieldCoords(/*GatePosition yellowGate, GatePosition blueGate*/);
	virtual void updateCoordinates(/*GatePosition yellowGate, GatePosition blueGate*/);
	double getAngle();
private:
	GatePosition & yellowGate, & blueGate; // use references that point somewhere
	void initPolarCoordinates();
	std::pair<cv::Point, cv::Point> intersectionOfTwoCircles(cv::Point circle1center, double circle1Rad, cv::Point circle2center, double circle2Rad);
	bool isRobotAboveCenterLine(double yellowGoalAngle, double blueGoalAngle);
	double getRobotDirection(cv::Point circle1center, cv::Point circle2center, double yellowGoalDist, double yellowGoalAngle, double blueGoalDist, double blueGoalAngle);
};
