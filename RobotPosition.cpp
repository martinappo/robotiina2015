#include "RobotPosition.h"
#include "DistanceCalculator.h"

extern DistanceCalculator gDistanceCalculator;

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

void RobotPosition::updateFieldCoordsNew(cv::Point2d orgin) {


	double d1 = blueGate.getDistance();
	double d2 = yellowGate.getDistance();
	double a1 = blueGate.getAngle();
	double a2 = yellowGate.getAngle();
	double a = a1 - a2;
	if (a < 0) a += 360;

	cv::Point pos;
	if (abs(a - 180) > 0){
		pos.x = a < 180 ? 1 : -1;
	}
	if (abs(d1 - d2) > 1){
		pos.y = d1 < d2 ? -1 : 1;
	}
	//fieldCoords = cv::Point(60 * pos.x, 100 * pos.y);
	double aa = a > 180 ? a - 180: a;
	double a11 = asin(d2 * sin(abs(aa / 180 * CV_PI)) / 460) / CV_PI * 180;
	double a12 = asin(d1 * sin(abs(aa / 180 * CV_PI)) / 460) / CV_PI * 180;
	double a111 = pos.x < 0 ? a11 : 180 - a11;
	double a112 = pos.y > 0 ? a11 : 180 - a11;
	double a121 = pos.x < 0 ? a12 : 180 - a12;
	double a122 = pos.y > 0 ? a12 : 180 - a12;
	double dx1 = d1 * sin(a111 / 180 * CV_PI)*pos.x;
	double dy1 = d1 * cos(a112 / 180 * CV_PI)*pos.y;
	double dx2 = d2 * sin(a121 / 180 * CV_PI)*pos.x;
	double dy2 = d2 * cos(a122 / 180 * CV_PI)*-pos.y;

	double x1 = dx1 + blueGate.fieldCoords.x;
	double y1 = dy1 + blueGate.fieldCoords.y;
	double x2 = dx2 + yellowGate.fieldCoords.x;
	double y2 = dy2 + yellowGate.fieldCoords.y;
//	x1 = x2; y1 = y2;
//	x2 = x1; y2 = y1;
	fieldCoords.x = (x1 + x2) / 2;
	fieldCoords.y = (y1 + y2) / 2;
	fieldCoords = filter.doFiltering(fieldCoords);
	// no that we know robot position, we can calculate it's angle to blue or yellow gate on the field
	double angleToBlueGate = DistanceCalculator::angleBetween(fieldCoords - blueGate.fieldCoords, { 0, -1 });
	double angleToYellowGate = DistanceCalculator::angleBetween(fieldCoords - yellowGate.fieldCoords, { 0, -1 });
	// now add real gate angle to this angle
	auto da1 = (angleToBlueGate - blueGate.getAngle());
	auto da2 = (angleToYellowGate - yellowGate.getAngle());
	// for taking average, they must have same sign
	if (abs(da1 - da2) > 180) {
		if (da1 < 0) da1 = 360 + da1;
		if (da2 < 0) da2 = 360 + da2;
	}
//	if (da2 < 0) da2 += 360;
	//polarMetricCoords.y = (da1 + da2) / 2;
	polarMetricCoords.y = d1 > d2 ? da1 : da2;

}
void RobotPosition::updateFieldCoords(cv::Point2d orgin) {

	updateFieldCoordsNew();
	return;

	auto possiblePoints = intersectionOfTwoCircles(yellowGate.fieldCoords, yellowGate.getDistance(), blueGate.fieldCoords, blueGate.getDistance());

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
	double angleToBlueGate = DistanceCalculator::angleBetween(fieldCoords - blueGate.fieldCoords, { 0, 1 });
	double angleToYellowGate = DistanceCalculator::angleBetween(fieldCoords - yellowGate.fieldCoords, { 0, 1 });
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
		circle1Rad += 0.2*circle1Rad;
		circle2Rad += 0.2*circle2Rad;
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