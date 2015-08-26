#pragma once
#include "DistanceCalibrator.h"

class DistanceCalculator{
public:
	DistanceCalculator(int centerX, int centerY);
	int getDistance(int x, int y);

private:
	int mCenterX;
	int mCenterY;
	void loadConf();
	int realDistances[DistanceCalibrator::CONF_SIZE];
	double references[DistanceCalibrator::CONF_SIZE];
};