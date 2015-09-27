#pragma once
#include "ObjectFinder.h"
class GateFinder :
	public ObjectFinder
{
public:
	GateFinder();
	~GateFinder();
	bool Locate(cv::Mat &thresholdedImage, cv::Mat &frameHSV, cv::Mat &frameBGR, ObjectPosition & objectPos);
};

