#pragma once
#include "DistanceCalibrator.h"

class DistanceCalculator{
public:
	DistanceCalculator();
	int getDistance(int centerX, int centerY, int x, int y);

private:
	void loadConf();
	int realDistances[DistanceCalibrator::CONF_SIZE];
	double references[DistanceCalibrator::CONF_SIZE];
};