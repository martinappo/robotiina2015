#include "WheelController.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <thread>


cv::Mat wheelAngles = (cv::Mat_<double>(4, 3) <<
	-sin(45.0 / 180 * CV_PI),  cos(45.0 / 180 * CV_PI), 1,
	-sin(135.0 / 180 * CV_PI), cos(135.0 / 180 * CV_PI), 1,
	-sin(225.0 / 180 * CV_PI), cos(225.0 / 180 * CV_PI), 1,
	-sin(315.0 / 180 * CV_PI), cos(315.0 / 180 * CV_PI), 1);


WheelController::WheelController(ISerial *port, int iWheelCount/* = 3*/) : m_iWheelCount(iWheelCount), m_pComPort(port)
{
	targetSpeed = { 0, 0, 0 };
	Drive(0);
};

void WheelController::DestroyWheels(){};

WheelController::~WheelController(){}

void WheelController::Forward(int speed) {
	DriveRotate(speed * 1.1547, 0, 0);
}

void WheelController::Rotate(bool direction, double speed){
	DriveRotate(0,0, direction ? speed : -speed);
}

void WheelController::Drive(double velocity, double direction, double rotate){
	DriveRotate(velocity, direction, rotate);
}

void WheelController::DriveRotate(double velocity, double direction, double rotate){
	if (abs(velocity) > 190) velocity = sign(velocity) * 190;
	if ((abs(rotate) + velocity) > 190) velocity = velocity - abs(rotate);


	targetSpeed.velocity = velocity; 
	targetSpeed.heading = direction; 
	targetSpeed.rotation = rotate;
	
	targetSpeedXYW.at<double>(0) = sin(direction* CV_PI / 180.0)* velocity;
	targetSpeedXYW.at<double>(1) = cos(direction* CV_PI / 180.0)* velocity;
	targetSpeedXYW.at<double>(2) = rotate;

	directControl = false;
	updateSpeed = true;
	sendCommand();
	
}

void WheelController::Drive(const cv::Point2d &speed, double angularSpeed){
	targetSpeedXYW.at<double>(0) = speed.x;
	targetSpeedXYW.at<double>(1) = speed.y;
	targetSpeedXYW.at<double>(2) = angularSpeed;
	sendCommand();
}

void WheelController::sendCommand(){
	if (IsStalled()) return;
	lastUpdate = boost::posix_time::microsec_clock::local_time();
	cv::Mat speeds = wheelAngles * targetSpeedXYW;
	std::ostringstream oss;
	for (auto i = 0; i < speeds.rows; i++) {
		oss << (i + id_start) << ":sd" << (int)speeds.at<double>(i) << "\n";
	}
	m_pComPort->WriteString(oss.str());
	std::this_thread::sleep_for(std::chrono::milliseconds(100)); // do not poll serial to fast
}

void WheelController::Stop(){
	Drive(0,0,0);
}

bool WheelController::IsStalled()
{
	if (directControl) return false;

	if (targetSpeed.velocity < 0.01 || actualSpeed.velocity > 90){
		stallTime = boost::posix_time::microsec_clock::local_time();
		return false;
	}
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type stallDuration = (time - stallTime).total_milliseconds();

		
	if (abs(actualSpeed.velocity - targetSpeed.velocity) < 10) {
		// reset timer
		stallTime = boost::posix_time::microsec_clock::local_time();
	}else {
		return stallDuration > 600;
	
	}
	return false;
}

bool WheelController::HasError(){	return false;// w_left->HasError() || w_right->HasError() || w_back->HasError();
}

std::vector<double> WheelController::GetWheelSpeeds()
{
	std::vector<double> speeds = std::vector<double>(m_iWheelCount);
	for (auto i = 0; i < m_iWheelCount; i++) {
		//TODO:FIXME speeds[i] = GetWheelTargetSpeed();
	}
	return speeds; //cv::Point3d(w_left->GetSpeed(), w_right->GetSpeed(), w_back->GetSpeed());
}



const Speed &  WheelController::GetTargetSpeed(){return targetSpeed;}

const Speed &  WheelController::GetActualSpeed(){return actualSpeed;}

std::string WheelController::GetDebugInfo(){
	std::ostringstream oss;
	oss.precision(4);
	oss << "[WheelController] target: " << "velocity: " << targetSpeed.velocity << ", heading: " << targetSpeed.heading << ", rotate: " << targetSpeed.rotation << "|";
	oss << "[WheelController] target: " << "vx: " << targetSpeedXYW.at<double>(0) << ", vy: " << targetSpeedXYW.at<double>(1) << ", rotate: " << targetSpeedXYW.at<double>(2) << "|";
	return oss.str();
}


