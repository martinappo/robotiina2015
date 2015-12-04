#pragma once
#include "ObjectFinder.h"
#include "FieldState.h"
#include "types.h"

class RobotFinder
{
public:
	RobotFinder();
	virtual ~RobotFinder();
	virtual bool Locate(cv::Mat &threshHoldedImage, cv::Mat &frameHSV, cv::Mat &frameBGR, std::vector<cv::Point2i> &objectCoords);
};

