#pragma once
#include "DistanceCalibrator.h"

class DistanceCalculator{
public:
	DistanceCalculator();
	double getDistance(int centerX, int centerY, int x, int y);
	void loadConf();

private:
	boost::property_tree::ptree pt;
	int realDistances[DistanceCalibrator::CONF_SIZE];
	double references[DistanceCalibrator::CONF_SIZE];
	std::atomic_bool m_bEnabled;
};