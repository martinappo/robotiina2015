#pragma once
#include "objectfinder.h"
#include "kalmanFilter.h"
class MouseFinder :
	public ObjectFinder
{
protected:
	virtual cv::Point2i LocateBallOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
public:
	MouseFinder();
	~MouseFinder();
private:
	cv::Point2i mouseLocation;
	KalmanFilter* filter = new KalmanFilter(cv::Point2i(40, 40));
};

