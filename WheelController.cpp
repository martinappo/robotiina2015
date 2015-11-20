#include "WheelController.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <thread>


cv::Mat wheelAngles = (cv::Mat_<double>(4, 3) <<
	-sin(45.0 / 180 * CV_PI),  cos(45.0 / 180 * CV_PI), 1,
	-sin(135.0 / 180 * CV_PI), cos(135.0 / 180 * CV_PI), 1,
	-sin(225.0 / 180 * CV_PI), cos(225.0 / 180 * CV_PI), 1,
	-sin(315.0 / 180 * CV_PI), cos(315.0 / 180 * CV_PI), 1);



//#define LIMIT_ACCELERATION

WheelController::WheelController(ISerial *port, int iWheelCount/* = 3*/) : ThreadedClass("WheelController"),
m_iWheelCount(iWheelCount), m_pComPort(port)
{
	targetSpeed = { 0, 0, 0 };
	Start();
	Drive(0);
};

void WheelController::DestroyWheels()
{

};
WheelController::~WheelController()
{
	Stop();
	WaitForStop();
}
void WheelController::Forward(int speed) {

	DriveRotate(speed * 1.1547, 0, 0);

}
void WheelController::Rotate(bool direction, double speed)
{
	DriveRotate(0,0, direction ? speed : -speed);
}
void WheelController::Drive(double velocity, double direction, double rotate)
{
	DriveRotate(velocity, direction, rotate);
}

void WheelController::DriveRotate(double velocity, double direction, double rotate)
{
 
	//std::cout << "DriveRotate: " << velocity << std::endl;

	if (abs(velocity) > 190){
		if (velocity > 0){
			velocity = 190;
		}
		else{
			velocity = -190;
		}
	}


	if ((abs(rotate) + velocity) > 190){
		if (rotate > 0){
			velocity = velocity - rotate;
		}
		else{
			velocity = velocity + rotate;
		}
			
	}
	//double sign = (velocity > 0) - (velocity < 0);
	//velocity = sign*std::min(190.0, abs(velocity + rotate)) - rotate;

	targetSpeed.velocity = velocity; // sin(direction* PI / 180.0)* velocity + rotate;
	targetSpeed.heading = direction; //cos(direction* PI / 180.0)* velocity + rotate,
	targetSpeed.rotation = rotate;
	
	targetSpeedXYW.at<double>(0) = sin(direction* CV_PI / 180.0)* velocity;
	targetSpeedXYW.at<double>(1) = cos(direction* CV_PI / 180.0)* velocity;
	targetSpeedXYW.at<double>(2) = rotate;

	directControl = false;
	updateSpeed = true;
	lastUpdate = boost::posix_time::microsec_clock::local_time();

	
}
void WheelController::Drive(const cv::Point2d &speed, double angularSpeed){
	targetSpeedXYW.at<double>(0) = speed.x;
	targetSpeedXYW.at<double>(1) = speed.y;
	targetSpeedXYW.at<double>(2) = angularSpeed;

}

void WheelController::Stop()
{
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
	//std::cout << "stall? " << velocity << ", " << velocity2 << "t: " << stallDuration << std::endl;

		
	if (abs(actualSpeed.velocity - targetSpeed.velocity) < 10) {
		// reset timer
		stallTime = boost::posix_time::microsec_clock::local_time();
	}else {
		//std::cout << "stall? " << velocity << ", " << velocity2 << "t: " << stallDuration << std::endl;
		return stallDuration > 600;
	
	}
	return false;

	//return w_left->IsStalled() || w_right->IsStalled() || w_back->IsStalled();
}
bool WheelController::HasError()
{
	return false;// w_left->HasError() || w_right->HasError() || w_back->HasError();
}

std::vector<double> WheelController::GetWheelSpeeds()
{
	std::vector<double> speeds = std::vector<double>(m_iWheelCount);
	for (auto i = 0; i < m_iWheelCount; i++) {
		//TODO:FIXME speeds[i] = GetWheelTargetSpeed();
	}

	return speeds; //cv::Point3d(w_left->GetSpeed(), w_right->GetSpeed(), w_back->GetSpeed());
}



const Speed &  WheelController::GetTargetSpeed()
{
	return targetSpeed;
}

const Speed &  WheelController::GetActualSpeed()
{
	return actualSpeed;
}

std::string WheelController::GetDebugInfo(){

	std::ostringstream oss;
	oss.precision(4);
	oss << "[WheelController] target: " << "velocity: " << targetSpeed.velocity << ", heading: " << targetSpeed.heading << ", rotate: " << targetSpeed.rotation << "|";
	oss << "[WheelController] target: " << "vx: " << targetSpeedXYW.at<double>(0) << ", vy: " << targetSpeedXYW.at<double>(1) << ", rotate: " << targetSpeedXYW.at<double>(2) << "|";
	/*
	cv::Point3d speeds = GetWheelSpeeds();
	auto speeds2 = CalculateWheelSpeeds(targetSpeed.velocity, targetSpeed.heading, targetSpeed.rotation);
	oss << "[Wheels] target: " << "left  : " << speeds2.x << ", right: " << speeds2.y << ", back: " << speeds2.z << "|";
	oss << "[Wheels] actual: " << "left  : " << speeds.x << ", right: " << speeds.y << ", back: " << speeds.x << "|";
	*/
	//oss << "[WheelController] pos: " << "x: " << robotPos.x << ", y: " << robotPos.y << ", r: " << robotPos.z;
	return oss.str();
}


void WheelController::Run()
{
	if (m_pComPort == NULL) return;
	while (!stop_thread) {
#ifdef LIMIT_ACCELERATION
		CalculateRobotSpeed();
		boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
		if (!updateSpeed && (now - lastUpdate).total_milliseconds() > 500) {
			targetSpeed = {0,0,0};
		}
		updateSpeed = false;
		Speed speed = targetSpeed;
		double dt = (double)(now - lastStep).total_milliseconds() / 1000.0;
		if (dt < 0.0000001) continue;

		if (abs(actualSpeed.velocity) > 1000) {
			std::cout << "to big actual velocity: " << actualSpeed.velocity << "wheel speeds: " << GetWheelSpeeds() << std::endl;
			actualSpeed.velocity = 0;

		}
		double dv = targetSpeed.velocity - actualSpeed.velocity;
		double sign = (dv > 0) - (dv < 0);
		double acc = dv / dt;

		acc = sign * std::min(fabs(acc), 500.0);
		speed.velocity = acc*dt + actualSpeed.velocity;

		auto speeds = CalculateWheelSpeeds(speed.velocity, speed.heading, speed.rotation);
		//std::cout << "wheel speeds, left: " << speeds.x << ", right: " << speeds.y << ", back: " << speeds.z << std::endl;
		w_left->SetSpeed(speeds.x);
		w_right->SetSpeed(speeds.y);
		w_back->SetSpeed(speeds.z);
		lastStep = now;
#else
		//auto speeds = CalculateWheelSpeeds(targetSpeed.velocity, targetSpeed.heading, targetSpeed.rotation);
		cv::Mat speeds = wheelAngles * targetSpeedXYW; 
		//std::cout << targetSpeedXYW << std::endl;
		std::ostringstream oss;
		for (auto i = 0; i < speeds.rows; i++) {
			oss << (i + id_start) << ":sd" << (int)speeds.at<double>(i) << "\n";
		}
		//std::cout << oss.str() << std::endl;
		m_pComPort->WriteString(oss.str());

#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // do not poll serial to fast
	}
	std::cout << "WheelController stoping" << std::endl;
	
}
