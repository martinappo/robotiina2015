#pragma once
#include "ObjectFinder.h"

class BallFinder :
	public ObjectFinder
{
public:
	int ballCountLeft=0;
	int ballCountRight=0;
	BallFinder();
	virtual ~BallFinder();
	cv::Point2i LocateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
	bool validateBall(ThresholdedImages &HSVRanges, cv::Point2d endPoint, cv::Mat &frameHSV, cv::Mat &frameBGR);
};

