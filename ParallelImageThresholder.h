#pragma once
#include "ImageThresholder.h"
class ParallelImageThresholder :
	public ImageThresholder
{
public:
	ParallelImageThresholder::ParallelImageThresholder(ThresholdedImages &images, HSVColorRangeMap &objectMap) : ImageThresholder(images, objectMap){
	}
	void Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList) {
		for (auto &object : objectList) {
			thresholdedImages[object] = cv::Mat(frameHSV.rows, frameHSV.cols, CV_8U, cv::Scalar::all(0));
		}
		auto &rb = objectMap[BALL];
		auto &rbg = objectMap[BLUE_GATE];
		auto &ryg = objectMap[YELLOW_GATE];
		
		auto &tb = thresholdedImages[BALL];
		auto &tbg = thresholdedImages[BLUE_GATE];
		auto &tyg = thresholdedImages[YELLOW_GATE];

		for (int i=0,k = 0; i < frameHSV.cols*frameHSV.rows * 3; i += 3, k++) {
			int h = frameHSV.data[i];
			int s = frameHSV.data[i + 1];
			int v = frameHSV.data[i + 2];



			bool ball = (rb.hue.low < h) && (rb.hue.high > h) && (rb.sat.low < s) && (rb.sat.high > s) && (rb.val.low < v) && (rb.val.high > v);
			bool blue = (rbg.hue.low < h) && (rbg.hue.high > h) && (rbg.sat.low < s) && (rbg.sat.high > s) && (rbg.val.low < v) && (rbg.val.high > v);
			bool yellow = (ryg.hue.low < h) && (ryg.hue.high > h) && (ryg.sat.low < s) && (ryg.sat.high > s) && (ryg.val.low < v) && (ryg.val.high > v);

			//frameHSV.data[i] = ball ? 255 : 0;
			//frameHSV.data[i + 1] = blue ? 255 : 0;
			//frameHSV.data[i + 2] = yellow ? 255 : 0;
			tb.data[k] = ball ? 255 : 0;
			tbg.data[k] = blue ? 255 : 0;
			tyg.data[k] = yellow ? 255 : 0;


		}
		//cv::extractChannel(frameHSV, thresholdedImages[BALL], 0);
		//cv::extractChannel(frameHSV, thresholdedImages[BLUE_GATE], 1);
		//cv::extractChannel(frameHSV, thresholdedImages[YELLOW_GATE], 2);
	}

};

