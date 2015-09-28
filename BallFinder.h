#pragma once
#include "ObjectFinder.h"
#include "FieldState.h"
#include "types.h"

class BallFinder
{
public:
	BallFinder();
	virtual ~BallFinder();
	virtual bool Locate(cv::Mat &threshHoldedImage, cv::Mat &frameHSV, cv::Mat &frameBGR, cv::Mat &objectCoords);
//	void populateBalls(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, FieldState *pFieldState);
	bool validateBall(ThresholdedImages &HSVRanges, cv::Point2d endPoint, cv::Mat &frameHSV, cv::Mat &frameBGR);
};

