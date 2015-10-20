#pragma once
#include "types.h"

class ObjectPosition
{
public:
	ObjectPosition(){};
	//ObjectPosition(const ObjectPosition& that) = delete; // disable copy positions
	double getDistance() const { return polarMetricCoords.x; };
	double getAngle() const { return polarMetricCoords.y; };
	cv::Point getFieldPos() { return fieldCoords; };
	virtual void updateRawCoordinates(const cv::Point pos, cv::Point orgin = cv::Point(0, 0)); // Takes raw coordinates of object from frame
	virtual ~ObjectPosition(){};

public:
	cv::Point2d fieldCoords; // (x, y) Coordinates to display objects on field by, relative to field
	cv::Point2i rawPixelCoords; // (x, y) Raw from frame
	cv::Point2d polarMetricCoords;      // (distance, angle) Relative to robot
	virtual void updateFieldCoords(cv::Point orgin = cv::Point(0, 0)) = 0;

protected:
	cv::Point2d lastFieldCoords;
};










