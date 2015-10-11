#pragma once
#include "types.h"
#include "ObjectPosition.h"

class GatePosition : public ObjectPosition
{
public:
	GatePosition(){};
	GatePosition(OBJECT gate);
	virtual ~GatePosition();
	virtual void updateFieldCoords(cv::Point orgin = cv::Point(0, 0)){};
};
