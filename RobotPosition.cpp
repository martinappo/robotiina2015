#include "RobotPosition.h"

//RobotPosition::RobotPosition() {
//	this->polarMetricCoords = cv::Point(0, 0);
//} 

RobotPosition::RobotPosition(GatePosition &yellowGate, GatePosition &blueGate, cv::Point initialCoords):
yellowGate(yellowGate), blueGate(blueGate), filter(initialCoords){
	this->polarMetricCoords = cv::Point(0, 0);
	this->fieldCoords = initialCoords;
}

RobotPosition::~RobotPosition()
{
}

void RobotPosition::updateFieldCoords(cv::Point orgin) {

	//we konw all angles of triangle and length of one side, use cosinus theorem to find
	const int distanceBetweenGates = 450;
	// we have two equations
	// distanceBetweenGates^2 = distanceToBlueGate^2 + distanceToYellowGate^2 - 2*distanceToBlueGate*distanceToYellowGate*cos(robotAngle)
	double diff = INT_MAX;
	double w = 1; 
	double a = 0.0001;//learning rate
	double d1 = blueGate.getDistance();
	double d2 = yellowGate.getDistance();
	double g = abs(blueGate.getAngle() - yellowGate.getAngle());
	if (true || abs(g - 180) > 15) { // do not use this for wery acute triangles, floating point errors are big
		while (d1 > 0 && d2 > 0 && abs(diff) > 1) {
			diff = distanceBetweenGates - sqrt(pow(w*d1, 2) + pow(w*d2, 2) - 2 * w*w*d1*d2*cos(g));
			w += diff*a;
		}
	}
	//w = 1;
	auto possiblePoints = intersectionOfTwoCircles(yellowGate.fieldCoords, w*d2, blueGate.fieldCoords,  w*d1);

	if (isRobotAboveCenterLine(yellowGate.getAngle(), blueGate.getAngle())){
		if (possiblePoints.first.y > 155){
			this->rawFieldCoords = possiblePoints.first;
		}else
			this->rawFieldCoords = possiblePoints.second;
	}
	else {
		if (possiblePoints.first.y < 155){
			this->rawFieldCoords = possiblePoints.first;
		}else
			this->rawFieldCoords = possiblePoints.second;
	}
	fieldCoords = filter.doFiltering(rawFieldCoords);
	/*double possiblePointDistance1 = cv::norm(possiblePoints.first - lastFieldCoords);
	double possiblePointDistance2 = cv::norm(possiblePoints.second - lastFieldCoords);
	if (possiblePointDistance1 < possiblePointDistance2) {
		this->fieldCoords = possiblePoints.first;
	}
	else {
		this->fieldCoords = possiblePoints.second;
	}*/

	// no that we know robot position, we can calculate it's angle to blue or yellow gate on the field
	double angleToBlueGate = angleBetween(fieldCoords - blueGate.fieldCoords, { 0, 1 });
	double angleToYellowGate = angleBetween(fieldCoords - yellowGate.fieldCoords, { 0, 1 });
	// now add real gate angle to this angle
	auto a1 = (angleToBlueGate - blueGate.getAngle());
	auto a2 = (angleToYellowGate - yellowGate.getAngle());
	// for taking average, they must have same sign
	if (a1 < 0) a1 += 360;
	if (a2 < 0) a2 += 360;
	//polarMetricCoords.y = (a1 + a2) / 2;
	polarMetricCoords.y = blueGate.getDistance() > yellowGate.getDistance() ? a1 : a2;


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

double RobotPosition::getAngle() {
	return polarMetricCoords.y;
}

bool RobotPosition::isRobotAboveCenterLine(double yellowGoalAngle, double blueGoalAngle){
	/*Calculation based on field: 
	  _________________
	 |                 |
	B|]-------o-------[|Y
	 |_________________|
	
	*/
	double yellowToBlue = blueGoalAngle - yellowGoalAngle;
	if (yellowToBlue < 0)
		yellowToBlue += 360;
	double blueToYellow = yellowGoalAngle - blueGoalAngle;
	if (blueToYellow < 0)
		blueToYellow += 360;
	if (yellowToBlue < blueToYellow)
		return true;
	return false;
}

//bluegoal 0 degrees, yellow 180 degrees
double RobotPosition::getRobotDirection(){

	// we have triangle and two conrners are known, subtract those from full circle
	return  ((int)(yellowGate.getAngle() - blueGate.getAngle()) % 360); // <- this is not correct
	
	// distance between the centers
	double distance = cv::norm(yellowGate.fieldCoords - blueGate.fieldCoords);
	double yellowGoalDist = yellowGate.getDistance();
	double blueGoalDist = blueGate.getDistance();

	// if two circle radiuses do not reach
	while (distance > yellowGoalDist + blueGoalDist) {
		yellowGoalDist++;
		blueGoalDist++;
	}

	double aSqr = blueGoalDist * blueGoalDist;
	double bSqr = 500.0 * 500.0;
	double cSqr = yellowGoalDist* yellowGoalDist;
	double ab2 = 2.0*blueGoalDist*500.0;
	double gammaCos = (aSqr + bSqr - cSqr) / ab2;
	double gammaRads = acos(gammaCos);
	double gammaDegrees = gammaRads*(180 / PI);
	double dir = gammaDegrees + (isRobotAboveCenterLine(yellowGate.getAngle(), blueGate.getAngle()) ? 0 : -360);
	return blueGate.getAngle() + dir;
	
}