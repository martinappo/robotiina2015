#pragma once
#include "ObjectFinder.h"
class GateFinder
{
public:
	GateFinder();
	~GateFinder();
	bool Locate(cv::Mat &thresholdedImage, cv::Mat &frameHSV, cv::Mat &frameBGR, cv::Point &center, cv::Point2f *bounds, std::vector<cv::Point2i> &notGates);
private:
	cv::Point2i getCenterFromContour(std::vector<cv::Point> contour);
};

