#include "DistanceCalculator.h"
#include "DistanceCalibrator.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
using namespace std;

string indent(int level) {
	string s;
	for (int i = 0; i<level; i++) s += "  ";
	return s;
}

void printTree(boost::property_tree::ptree &pt, int level) {
	if (pt.empty()) {
		cerr << "\"" << pt.data() << "\"";
	}
	else {
		if (level) cerr << endl;
		cerr << indent(level) << "{" << endl;
		for (boost::property_tree::ptree::iterator pos = pt.begin(); pos != pt.end();) {
			cerr << indent(level + 1) << "\"" << pos->first << "\": ";
			printTree(pos->second, level + 1);
			++pos;
			if (pos != pt.end()) {
				cerr << ",";
			}
			cerr << endl;
		}
		cerr << indent(level) << " }";
	}
	return;
}

DistanceCalculator::DistanceCalculator(){
	loadConf();
}

void DistanceCalculator::loadConf(){
	m_bEnabled = false;
	try{
		read_ini("distance_conf.ini", pt);
		//printTree(pt, 0);
		/*
		int confKey = DistanceCalibrator::DISTANCE_CALIBRATOR_STEP;
		for (int i = 0; i < DistanceCalibrator::CONF_SIZE; i++){
			std::ostringstream key;
			key << confKey;
			realDistances[i] = confKey;
			references[i] = pt.get<double>(key.str()); 
			confKey += DistanceCalibrator::DISTANCE_CALIBRATOR_STEP;
		}
		*/
	}
	catch (std::exception const&  ex)
	{
		std::cout << "Can't init settings" << ex.what()<<std::endl;
	}
	m_bEnabled = true;
}

double DistanceCalculator::getDistance(int centerX, int centerY, int x, int y){
	if (!m_bEnabled) return INT_MAX;
	double dist = cv::norm(cv::Point(centerX, centerY) - cv::Point(x, y));//DistanceCalibrator::calculateDistance(centerX, centerY, x, y);
	//y = 7E-10x5 - 8E-07x4 + 0,0004x3 - 0,0782x2 + 7,6694x - 239,79
	//return 7E-10*pow(dist, 5) - 8E-07*pow(dist, 4) + 0.0004*pow(dist, 3) - 0.0782*pow(dist, 2) + 7.6694*dist - 239.79;

	//y = 13,136e^0,008x
	return 13.13*exp(0.008 * dist);

	double minDif = INT_MAX;
	int index = 0;
	auto itCur = pt.begin();
	if (itCur == pt.end()) { // no conf
		return INT_MAX;
	}
	double prev1 = atof(itCur->first.c_str());
	double prev2 = atof(itCur->second.data().c_str());

	for (itCur++; itCur != pt.end(); itCur++){
	//for (int i = 1; i < DistanceCalibrator::CONF_SIZE; i++){
		double next1 = atof(itCur->first.c_str());
		double next2 = atof(itCur->second.data().c_str());
		if (next2 > dist) {
			double how_far_between_two_steps = (dist - prev2) / (next2 - prev2);
			double realdist = prev1 + (next1 - prev1)*how_far_between_two_steps;
			//std::cout << "distance " << dist << " -> " << realdist << std::endl;
			return realdist;
		}
		prev1 = next1;
		prev2 = next2;
		/*
		double dif = std::abs(references[i] - dist);
		if (dif < minDif){
			index = i;
			minDif = dif;
		}
		*/
	}
}
