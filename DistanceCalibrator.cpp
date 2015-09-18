#include "DistanceCalibrator.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <chrono>
#include <thread>

DistanceCalibrator::DistanceCalibrator(ICamera * pCamera, IDisplay *pDisplay){
	m_pCamera = pCamera;
	m_pDisplay = pDisplay;
	frame_size = m_pCamera->GetFrameSize();
	counterValue = "NA";
	m_pDisplay->AddEventListener(this);
	points.push_back(std::make_pair<std::string, cv::Point>(std::string("Click on blue gate on camera image"), cv::Point(0, 0)));
	points.push_back(std::make_pair<std::string, cv::Point>(std::string("Click on yellow gate on camera image"), cv::Point(0, 0)));
	points.push_back(std::make_pair<std::string, cv::Point>(std::string("Click on robot location in field"), cv::Point(0, 0)));

};

bool DistanceCalibrator::OnMouseEvent(int event, float x, float y, int flags, bool bMainArea) {
	if (enabled && event == cv::EVENT_LBUTTONUP) {
		itPoints->second = cv::Point(x, y);
		itPoints++;
		if(itPoints == points.end()){
			calculateDistances();
			message = "press backspace, calibration data saved to console";
			enabled = false;
		}
		else {
			message = itPoints->first;
		}
		return true;
		if (bMainArea)
			mouseClicked(x, y, flags);
		else if(!bMainArea)
			mouseClicked2(x, y, flags);
		return true;
	}
	return false;

};
void DistanceCalibrator::calculateDistances(){
	cv::Point2d blue1 = points[0].second;
	cv::Point2d yellow1 = points[1].second;
	cv::Point2d self1 = cv::Point2d(frame_size) / 2;

	cv::Point blue2(0, -250);
	cv::Point yellow2(0, 250);
	cv::Point self2 = points[2].second - cv::Point(304,304);
	
	double distanceToBlue1 = cv::norm(blue1 - self1);
	double distanceToYellow1 = cv::norm(yellow1 - self1);
	std::cout << "Blue 1: " << distanceToBlue1 << std::endl;
	std::cout << "Yellow 1: " << distanceToYellow1 << std::endl;

	double distanceToBlue2 = cv::norm(blue2 - self2);
	double distanceToYellow2 = cv::norm(yellow2 - self2);
	std::cout << "Blue 2: " << distanceToBlue2 << std::endl;
	std::cout << "Yellow 2: " << distanceToYellow2 << std::endl;
	std::cout << "======begin copy/paste============" << std::endl;
	std::cout << distanceToBlue2 << "=" << distanceToBlue1 << std::endl;
	std::cout << distanceToYellow2 << "=" << distanceToYellow1 << std::endl;
	std::cout << "======end copy/paste============" << std::endl;


}

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
	counterValue = valueString;
	std::cout << counter << std::endl;
	if (counter == VIEWING_DISTANCE){
		enabled = false;
		boost::property_tree::write_ini("distance_conf.ini", pt);
	}
	return;
}

void DistanceCalibrator::mouseClicked2(int x, int y, int flags) {
	std::cout << x << ", " << y 
		<< ", d1: " << cv::norm(cv::Point(x,y)- cv::Point(304, 72))
		<< ", d2: " << cv::norm(cv::Point(x, y)- cv::Point(304, 534)) << std::endl;
	
}
void DistanceCalibrator::Enable(bool enable){
	enabled = enable;
	counterValue = "NA";
	counter = 0;
	pt.clear();
	itPoints = points.begin();
	message = enable ? itPoints->first : "";

}


DistanceCalibrator::~DistanceCalibrator(){
	m_pDisplay->RemoveEventListener(this);
}

