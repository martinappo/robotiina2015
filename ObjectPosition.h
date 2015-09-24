#pragma once
#include "types.h"

class ObjectPosition : public IObjectPosition
{
public:
	ObjectPosition(){};
	ObjectPosition(const ObjectPosition& that) = delete; // disable copy positions
	ObjectPosition(int distance, int angle);
	ObjectPosition(cv::Point2i polarCoords);
	virtual ~ObjectPosition();
	double getDistance();
	void setDistance(int distance);
	double getAngle();
	double getAngleToRobot();
	double robotAngle;
	void setFrameSize(cv::Size frameSize);

	virtual void updateCoordinates(int x, int y, cv::Point robotFieldCoords, double robotAngle); // Takes raw coordinates of object from frame
	virtual void updateCoordinates(cv::Point point, cv::Point robotFieldCoords, double robotAngle);
	virtual void updatePolarCoords(int x, int y);
	virtual void updatePolarCoords(cv::Point point);

	cv::Point2i fieldCoords; // (x, y) Coordinates to display objects on field by, relative to field
	cv::Point2i rawPixelCoords; // (x, y) Raw from frame
	cv::Point2d polarMetricCoords;      // (distance, angle) Relative to robot
	
protected:
	virtual void updatePolarCoords();
	virtual void updateFieldCoords(cv::Point robotFieldCoords);
	cv::Point2i lastFieldCoords;
private:
	cv::Size frameSize;
	cv::Point2i center = { frameSize.width / 2, frameSize.height / 2};
	double angleBetween(const cv::Point2i &p1, const cv::Point2i &p2);

};










