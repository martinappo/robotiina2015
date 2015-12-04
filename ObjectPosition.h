#pragma once
#include "types.h"

class ObjectPosition
{
public:
	ObjectPosition(){};
	//ObjectPosition(const ObjectPosition& that) = delete; // disable copy positions
	double getDistance() const { return polarMetricCoords.x; };
	double getAngle() const { return polarMetricCoords.y; };
	double getHeading() const { 
		if(polarMetricCoords.y > 0)
			return polarMetricCoords.y > 180 ? polarMetricCoords.y -360 : polarMetricCoords.y; 
		else
			return polarMetricCoords.y < -180 ? polarMetricCoords.y +360 : polarMetricCoords.y; 

	};
	cv::Point2d getFieldPos() { return fieldCoords; };
	virtual void updateRawCoordinates(const cv::Point2d pos, cv::Point2d orgin = cv::Point2d(0, 0)); // Takes raw coordinates of object from frame
	virtual ~ObjectPosition(){};

public:
	cv::Point2d fieldCoords = cv::Point2d(INT_MAX, INT_MAX); // (x, y) Coordinates to display objects on field by, relative to field
	cv::Point2i rawPixelCoords; // (x, y) Raw from frame
	cv::Point2d polarMetricCoords;      // (distance, angle) Relative to robot
	cv::Point2i filteredRawCoords; //filtered raw coords (almost as cartesian absolute pos)
	virtual void updateFieldCoords(cv::Point2d orgin = cv::Point2d(0, 0)) {
		throw std::runtime_error("Not implemented");
	};

protected:
	cv::Point2d lastFieldCoords = cv::Point2d(INT_MAX, INT_MAX);;
};









