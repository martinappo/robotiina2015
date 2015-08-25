#include "DistanceCalculator.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

DistanceCalculator::DistanceCalculator(int centerX, int centerY){
	mCenterX = centerX;
	mCenterY = centerY;
	loadConf();
}

void DistanceCalculator::loadConf(){
	using boost::property_tree::ptree;

	ptree pt;
	try{
		read_ini("distance_conf.ini", pt);
		int confKey = DistanceCalibrator::DISTANCE_CALIBRATOR_STEP;
		for (int i = 0; i <= DistanceCalibrator::CONF_SIZE; i++){
			std::ostringstream key;
			key << confKey;
			realDistances[i] = confKey;
			references[i] = pt.get<double>(key.str()); 
		}
	}
	catch (...){};
}

int DistanceCalculator::getDistance(int x, int y){
	double dist = DistanceCalibrator::calculateDistance(DistanceCalculator::mCenterX, DistanceCalculator::mCenterY, x, y);
	double minDif = 9999999999999999999;
	int index = 0;
	for (int i = 0; i < DistanceCalibrator::CONF_SIZE; i++){
		double dif = std::abs(references[i] - dist);
		if (dif < minDif){
			index = i;
			minDif = dif;
		}
	}
	return realDistances[index];
}