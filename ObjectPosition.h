#pragma once
#include "types.h"

class ObjectPosition : public IObjectPosition
{
public:
#ifdef WIN32
	ObjectPosition() {};
#else
	ObjectPosition() noexcept {};
#endif
	ObjectPosition(int distance, int angle);
	ObjectPosition(cv::Point2i polarCoords);
	virtual ~ObjectPosition();
	int getDistance();
	void setDistance(int distance);
	int getAngle();
	virtual void updateCoordinates(int x, int y); // Takes raw coordinates of object from frame
	virtual void updatePolarCoords(int x, int y);
	virtual void updateMetricCoords(int x, int y);
	virtual void updateFieldPixelCoords(int x, int y);
	virtual void updateCoordinates(cv::Point point);
	cv::Point2i pixelCoordsForField; // (x, y) Coordinates to display objects on field by, relative to field
	cv::Point2i rawPixelCoords; // (x, y) Raw from frame
	cv::Point2i polarPixelCoords;
	cv::Point2i polarMetricCoords;      // (distance, angle) Relative to robot
	cv::Point2i metricCoords;     // (x, y) Real object coordinates relative to field in mm
	cv::Size frameSize;
private:
	cv::Point2i center = { frameSize.width / 2, frameSize.height / 2};
	double angleBetween(const cv::Point2i &p1, const cv::Point2i &p2, const cv::Point2i &p3);
};










