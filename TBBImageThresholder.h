#pragma once
#include "types.h"
#define USE_INRANGE
class TBBImageThresholder :
	public ImageThresholder, public cv::ParallelLoopBody
{
public:
	TBBImageThresholder(ThresholdedImages &images, HSVColorRangeMap &objectMap) : ImageThresholder(images, objectMap){
	}

	void Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList) {
		this->frameHSV = frameHSV;
		this->objectList = objectList;
		for (auto &object : objectList) {
			thresholdedImages[object] = cv::Mat(frameHSV.rows, frameHSV.cols, CV_8U, cv::Scalar::all(0));
		}
		cv::parallel_for_(cv::Range(0, diff), *this);
		/*
		cv::extractChannel(frameHSV, thresholdedImages[BALL], 0);
		cv::extractChannel(frameHSV, thresholdedImages[BLUE_GATE], 1);
		cv::extractChannel(frameHSV, thresholdedImages[YELLOW_GATE], 2);
		*/
	}

	virtual void operator()(const cv::Range& range) const
	{
		auto &rb = objectMap[BALL];
		auto &rbg = objectMap[BLUE_GATE];
		auto &ryg = objectMap[YELLOW_GATE];
		auto &tb = thresholdedImages[BALL];
		auto &tbg = thresholdedImages[BLUE_GATE];
		auto &tyg = thresholdedImages[YELLOW_GATE];

		for (int i = range.start; i < range.end; i++)
		{

#ifndef USE_INRANGE
			for (int j = (frameHSV.cols*frameHSV.rows * 3 / diff)*i, k = (frameHSV.cols*frameHSV.rows / diff)*i; j < (frameHSV.cols*frameHSV.rows * 3 / diff)*(i + 1); j += 3, k++) {
				//if (j % 2) continue;
				int h = frameHSV.data[j];
				int s = frameHSV.data[j + 1];
				int v = frameHSV.data[j + 2];


				bool ball = (rb.hue.low <= h) && (rb.hue.high >= h) && (rb.sat.low <= s) && (rb.sat.high >= s) && (rb.val.low <= v) && (rb.val.high >= v);
				bool blue = (rbg.hue.low <= h) && (rbg.hue.high >= h) && (rbg.sat.low <= s) && (rbg.sat.high >= s) && (rbg.val.low <= v) && (rbg.val.high >= v);
				bool yellow = (ryg.hue.low <= h) && (ryg.hue.high >= h) && (ryg.sat.low <= s) && (ryg.sat.high >= s) && (ryg.val.low <= v) && (ryg.val.high >= v);

				//frameHSV.data[j] = ball ? 255 : 0;
				//frameHSV.data[j + 1] = blue ? 255 : 0;
				//frameHSV.data[j + 2] = yellow ? 255 : 0;
				tb.data[k] = ball ? 255 : 0;
				tbg.data[k] = blue ? 255 : 0;
				tyg.data[k] = yellow ? 255 : 0;


			}


#else // use inRange
			cv::Mat in(frameHSV, cv::Rect(0, (frameHSV.rows / diff)*i,
			frameHSV.cols, frameHSV.rows / diff));

			for (auto &object : objectList) {
				auto r = objectMap[object];
				cv::Mat out(thresholdedImages[object], cv::Rect(0, (thresholdedImages[object].rows / diff)*i,
					thresholdedImages[object].cols, thresholdedImages[object].rows / diff));
				inRange(in, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), out);
			}
#endif
		}
	}
protected:
	cv::Mat frameHSV;
	std::vector<OBJECT> objectList;
	int diff = 12;
};

