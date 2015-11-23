#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "GatePosition.h"
#include "KalmanFilter.h"
class RobotPosition : public ObjectPosition
{
public:
	RobotPosition(GatePosition &yellowGate, GatePosition &blueGate, cv::Point initialCoords = cv::Point(0,0));
	virtual ~RobotPosition();
	virtual void updatePolarCoords();
	void updateFieldCoordsNew(cv::Point2d orgin = cv::Point(0, 0));
	void updateFieldCoords(cv::Point2d orgin = cv::Point(0, 0));
	double getAngle();
	cv::Point2d rawFieldCoords; // (x, y) Coordinates to display objects on field by, relative to field
private:
	GatePosition & yellowGate, & blueGate; // use references that point somewhere
	void initPolarCoordinates();
	std::pair<cv::Point, cv::Point> intersectionOfTwoCircles(cv::Point circle1center, double circle1Rad, cv::Point circle2center, double circle2Rad);
	bool isRobotAboveCenterLine(double yellowGoalAngle, double blueGoalAngle);
	double getRobotDirection();
	KalmanFilter filter;
	double tmp;
};
