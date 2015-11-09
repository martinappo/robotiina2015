#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>

class KalmanFilter {
protected:
	cv::KalmanFilter KF = cv::KalmanFilter(4, 2, 0);
	cv::Mat_<float> measurement = cv::Mat_<float>(2, 1);
	cv::Mat_<float> estimated = cv::Mat_<float>(2, 1);
	int predictCount = 0;
public:
	KalmanFilter(const cv::Point2i &startPoint);
	cv::Point2d doFiltering(const cv::Point2i &point);
	cv::Point2d getPrediction();
	void reset(const cv::Point2i &point);
	
};
