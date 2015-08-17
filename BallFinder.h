#pragma once
#include "ObjectFinder.h"
#include "FieldState.h"
#include "types.h"

class BallFinder :
	public ObjectFinder
{
public:
	BallFinder();
	virtual ~BallFinder();
	void BallFinder::PopulateBalls(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, FieldState *pFieldState);
	bool validateBall(ThresholdedImages &HSVRanges, cv::Point2d endPoint, cv::Mat &frameHSV, cv::Mat &frameBGR);
};

