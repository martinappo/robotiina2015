#include "DistanceCalculator.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

DistanceCalculator::DistanceCalculator(){
	loadConf();
}

void DistanceCalculator::loadConf(){
	using boost::property_tree::ptree;

	ptree pt;
	try{
		read_ini("distance_conf.ini", pt);
		int confKey = DistanceCalibrator::DISTANCE_CALIBRATOR_STEP;
		for (int i = 0; i < DistanceCalibrator::CONF_SIZE; i++){
			std::ostringstream key;
			key << confKey;
			realDistances[i] = confKey;
			references[i] = pt.get<double>(key.str()); 
			confKey += DistanceCalibrator::DISTANCE_CALIBRATOR_STEP;
		}
	}
	catch (...){};
}

int DistanceCalculator::getDistance(int centerX, int centerY, int x, int y){
	double dist = DistanceCalibrator::calculateDistance(centerX, centerY, x, y);
	double minDif = INT_MAX;
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
