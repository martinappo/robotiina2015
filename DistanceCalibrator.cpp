#include "DistanceCalibrator.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <chrono>
#include <thread>

DistanceCalibrator::DistanceCalibrator(ICamera * pCamera, IDisplay *pDisplay){
	m_pCamera = pCamera;
	m_pDisplay = pDisplay;
	frame_size = m_pCamera->GetFrameSize();
};

bool DistanceCalibrator::OnMouseEvent(int event, float x, float y, int flags) {
	if (event == cv::EVENT_LBUTTONUP && x < 1.0 && y < 1.0 && (x > 0.2 || y > 0.2) && distanceCalibrationRunning) {
		mouseClicked(x*frame_size.x, y*frame_size.y, flags);
		return true;
	}
	return false;

};

double DistanceCalibrator::calculateDistance(double centreX, double centreY, double x, double y){
	return std::sqrt(std::abs(centreX - x)*std::abs(centreX - x) + std::abs(centreY - y)*std::abs(centreY - y));
}

void DistanceCalibrator::mouseClicked(int x, int y, int flags) {
	//std::cout << x << ", " << y << "--" << frame_size.x / 2 << ", " <<frame_size.y / 2 <<"  " << counter << "  dist: " << calculateDistance(frame_size.x / 2, frame_size.y / 2, x, y) << std::endl;
	counter = counter + DistanceCalibrator::DISTANCE_CALIBRATOR_STEP;
	std::ostringstream distance, value;
	distance << calculateDistance(frame_size.x / 2, frame_size.y / 2, x, y);
	value << counter;
	std::string distanceString = distance.str();
	std::string valueString = value.str();
	pt.put(valueString, distanceString);

	if (counter == VIEWING_DISTANCE){
		distanceCalibrationRunning = false;
		boost::property_tree::write_ini("distance_conf.ini", pt);
	}
	return;
}

void DistanceCalibrator::start(){
	distanceCalibrationRunning = true;
	counter = 0;
	pt.clear();
	m_pDisplay->AddEventListener(this);
}

void DistanceCalibrator::removeListener(){
	m_pDisplay->RemoveEventListener(this);
	distanceCalibrationRunning = false;
}

DistanceCalibrator::~DistanceCalibrator(){
	removeListener();
}

