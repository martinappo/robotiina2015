#include "RobotPosition.h"

//RobotPosition::RobotPosition() {
//	this->polarMetricCoords = cv::Point(0, 0);
//} 

RobotPosition::RobotPosition(GatePosition yellowGate, GatePosition blueGate, cv::Point initialCoords){
	this->polarMetricCoords = cv::Point(0, 0);
	this->fieldCoords = initialCoords;
	updateCoordinates(yellowGate, blueGate);
}

RobotPosition::~RobotPosition()
{
}

void RobotPosition::updateCoordinates(GatePosition yellowGate, GatePosition blueGate) {
	lastFieldCoords = fieldCoords;
	this->robotAngle = yellowGate.getAngle();
	updateFieldCoords(yellowGate, blueGate);
}

void RobotPosition::updateFieldCoords(GatePosition yellowGate, GatePosition blueGate) {
	std::pair<cv::Point, cv::Point> possiblePoints = intersectionOfTwoCircles(yellowGate.fieldCoords, 
																			  yellowGate.getDistance(), 
																			  blueGate.fieldCoords, 
																			  blueGate.getDistance());
	double possiblePointDistance1 = cv::norm(possiblePoints.first - lastFieldCoords);
	double possiblePointDistance2 = cv::norm(possiblePoints.second - lastFieldCoords);
	if (possiblePointDistance1 < possiblePointDistance2) {
		this->fieldCoords = possiblePoints.first;
	}
	else {
		this->fieldCoords = possiblePoints.second;
	}
}

void RobotPosition::updatePolarCoords() {
	return;
}

std::pair<cv::Point, cv::Point> RobotPosition::intersectionOfTwoCircles(cv::Point circle1center, 
																		   double circle1Rad, 
																		cv::Point circle2center, 
																		   double circle2Rad) {
	// distance between the centers
	double distance = cv::norm(circle1center - circle2center);

	// if two circle radiuses do not reach
	while (distance > circle1Rad + circle2Rad) {
		circle1Rad++;
		circle2Rad++;
	}

	// calculating area and height of trianlge formed by points
	double a = (pow(circle1Rad,2) - pow(circle2Rad, 2) + pow(distance, 2)) / (2.0*distance);
	double h = sqrt(pow(circle1Rad, 2) - pow(a, 2));

	//Calculate point p, where the line through the circle intersection points crosses the line between the circle centers.  
	cv::Point p;

	p.x = (int)(circle1center.x + (a / distance) * (circle2center.x - circle1center.x));
	p.y = (int)(circle1center.y + (a / distance) * (circle2center.y - circle1center.y));

	// if has only one intersection point
	if (distance == circle1Rad + circle2Rad) {
		return std::pair<cv::Point, cv::Point>(p, p);
	}

	// if has two intersection points
	cv::Point possible1;
	cv::Point possible2;

	possible1.x = (int)(p.x + (h / distance) * (circle2center.y - circle1center.y));
	possible1.y = (int)(p.y - (h / distance) * (circle2center.x - circle1center.x));

	possible2.x = (int)(p.x - (h / distance) * (circle2center.y - circle1center.y));
	possible2.y = (int)(p.y + (h / distance) * (circle2center.x - circle1center.x));

	return std::pair<cv::Point, cv::Point>(possible1, possible2);
}

int RobotPosition::getAngle() {
	return robotAngle;
}