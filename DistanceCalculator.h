#pragma once
#include "DistanceCalibrator.h"

class DistanceCalculator{
public:
	DistanceCalculator();
	double getDistance(int centerX, int centerY, int x, int y);

private:
	boost::property_tree::ptree pt;
	void loadConf();
	int realDistances[DistanceCalibrator::CONF_SIZE];
	double references[DistanceCalibrator::CONF_SIZE];
};