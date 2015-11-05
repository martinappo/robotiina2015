#pragma once
#include "DistanceCalibrator.h"

class DistanceCalculator{
public:
	DistanceCalculator();
	double getDistance(const cv::Point2d &pos, const cv::Point2d &orgin) const;
	double getDistanceInverted(const cv::Point2d &pos, const cv::Point2d &orgin) const;
	cv::Point2d getPolarCoordinates(const cv::Point2d &pos, const cv::Point2d &orgin) const;
	cv::Point2d getFieldCoordinates(const cv::Point2d &pos, const cv::Point2d &orgin) const;
	void loadConf();

	static double angleBetween(const cv::Point2d &a, const cv::Point2d &b) {
		double alpha = atan2(a.y, a.x) - atan2(b.y, b.x);
		double alphaDeg = alpha * 180. / CV_PI;
		if (alphaDeg < 0) alphaDeg += 360;
		return alphaDeg;
	}

private:
	boost::property_tree::ptree pt;
	int realDistances[DistanceCalibrator::CONF_SIZE];
	double references[DistanceCalibrator::CONF_SIZE];
	std::atomic_bool m_bEnabled;
};