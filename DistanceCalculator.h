#pragma once
#include "DistanceCalibrator.h"

class DistanceCalculator{
public:
	DistanceCalculator();
	double getDistance(const cv::Point &pos, const cv::Point &orgin) const;
	void loadConf();

private:
	boost::property_tree::ptree pt;
	int realDistances[DistanceCalibrator::CONF_SIZE];
	double references[DistanceCalibrator::CONF_SIZE];
	std::atomic_bool m_bEnabled;
};