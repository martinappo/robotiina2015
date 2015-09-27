#pragma once
#include "types.h"

class ObjectPosition
{
public:
	ObjectPosition(){};
	ObjectPosition(const ObjectPosition& that) = delete; // disable copy positions
	double getDistance() const { return polarMetricCoords.x; };
	double getAngle() const { return polarMetricCoords.y; };
	cv::Point getFieldPos();
	virtual void updateRawCoordinates(const cv::Point pos, cv::RotatedRect bounds, cv::Point orgin = cv::Point(0, 0)); // Takes raw coordinates of object from frame
	virtual ~ObjectPosition();
	ObjectPosition(int distance, int angle);
	ObjectPosition(cv::Point2i polarCoords);

private:
	void setDistance(int distance);
	double getAngleToRobot();
	double robotAngle;
	//void setFrameSize(cv::Size frameSize);

	virtual void updateCoordinates(int x, int y, cv::Point robotFieldCoords, double robotAngle); // Takes raw coordinates of object from frame
	virtual void updateCoordinates(cv::Point point, cv::Point robotFieldCoords, double robotAngle);
	virtual void updatePolarCoords(int x, int y);
	virtual void updatePolarCoords(cv::Point point);
public:
	cv::Point2i fieldCoords; // (x, y) Coordinates to display objects on field by, relative to field
	cv::Point2i rawPixelCoords; // (x, y) Raw from frame
	cv::Point2d polarMetricCoords;      // (distance, angle) Relative to robot
	cv::RotatedRect rawBounds; // object bounds on raw image
	
protected:
	virtual void updatePolarCoords();
	virtual void updateFieldCoords(cv::Point robotFieldCoords);
	cv::Point2i lastFieldCoords;
private:
	//cv::Size frameSize;
	//cv::Point2i center = { frameSize.width / 2, frameSize.height / 2};
protected:
	double angleBetween(const cv::Point2i &p1, const cv::Point2i &p2);

};










