#pragma once
#include "types.h"
#include "ObjectPosition.h"

class GatePosition : public ObjectPosition
{
public:
	GatePosition(){};
	GatePosition(OBJECT gate);
	virtual ~GatePosition();
	virtual void updateFieldCoords(cv::Point2d orgin = cv::Point2d(0, 0)){};
	cv::Point2d minCornerPolarCoords;
};
