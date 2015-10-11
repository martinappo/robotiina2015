#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "TargetPosition.h"
#include "BallPosition.h"

class TargetPosition : public ObjectPosition
{
public:
	TargetPosition(){};
	TargetPosition(cv::Point orgin);
	TargetPosition(BallPosition orgin);
	virtual ~TargetPosition();
	void updateFieldCoords(cv::Point orgin);
};