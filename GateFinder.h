#pragma once
#include "ObjectFinder.h"
class GateFinder
{
public:
	GateFinder();
	~GateFinder();
	bool Locate(cv::Mat &thresholdedImage, cv::Mat &frameHSV, cv::Mat &frameBGR, cv::Point &center, cv::Point2f *bounds);
};

