#pragma once
#include "objectfinder.h"
class GateFinder :
	public ObjectFinder
{
public:
	GateFinder();
	~GateFinder();
	cv::Point2i LocateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
};

