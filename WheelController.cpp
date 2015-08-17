#include "WheelController.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <thread>

#define deg150 (150.0 * PI / 180.0)
#define deg30 (30.0 * PI / 180.0)
#define deg270 (270.0 * PI / 180.0)
//#define LIMIT_ACCELERATION

WheelController::WheelController(ComPortScanner *pScanner, int iWheelCount/* = 3*/) : ThreadedClass("WheelController")
, m_pScanner(pScanner), m_iWheelCount(iWheelCount)
{
	if (iWheelCount == 3) {
		m_vWheels.push_back(std::make_pair<double, SimpleSerial*>(150, NULL));
		m_vWheels.push_back(std::make_pair<double, SimpleSerial*>(30, NULL));
		m_vWheels.push_back(std::make_pair<double, SimpleSerial*>(270, NULL));
	}
//	for (auto i = 0; i < iWheelCount; i++) {
//		m_vWheels.push_back(std::make_pair<double, SimpleSerial*>((double)i / m_iWheelCount, NULL));
//	}
	targetSpeed = { 0, 0, 0 };
	m_bPortsInitialized = false;
	Start();
};


void WheelController::Init()
{
	if (m_pScanner == NULL || m_bPortsInitialized) return;
	using boost::property_tree::ptree;
	ptree pt;
	try {
		read_ini("conf/wheels.ini", pt);

		bool bPortsOk = true;
		for (auto i = 0; i < m_iWheelCount; i++) {
			auto sPortNum = pt.get<std::string>(std::to_string(i+1));
			SimpleSerial * pPort = NULL;
			if (m_pScanner->VerifyPort(sPortNum, i + 1, &pPort)) {
				m_vWheels[i].second = pPort;
			}else{
				bPortsOk = false;
			}
		}
		m_bPortsInitialized = bPortsOk;
	}
	catch (std::runtime_error const&e) {
		std::cout << "Error reading wheel configuration: " << e.what() << std::endl;
	}
	if (!m_bPortsInitialized) {
		std::vector<int> ids;
		for (auto i = 0; i < m_iWheelCount; i++) {
			ids.push_back(i+1);
		}
		m_pScanner->ScanObject("conf/wheels.ini", ids);
	}
}

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
#ifndef LIMIT_ACCELERATION
	auto speeds = CalculateWheelSpeeds(targetSpeed.velocity, targetSpeed.heading, targetSpeed.rotation);
	for (auto i = 0; i < m_iWheelCount; i++) {
		if (m_vWheels[i].second != NULL) {
			std::ostringstream oss;
			oss << "sd" << speeds[i] << "\n";
			m_vWheels[i].second->writeString(oss.str());
		}
	}
	//if (w_left != NULL) w_left->SetSpeed(speeds.x);
	//if (w_right != NULL) w_right->SetSpeed(speeds.y);
	//if (w_back != NULL) w_back->SetSpeed(speeds.z);
#endif
	directControl = false;
	updateSpeed = true;
	lastUpdate = boost::posix_time::microsec_clock::local_time();

	
}
std::vector<double> WheelController::CalculateWheelSpeeds(double velocity, double direction, double rotate)
{
	std::vector<double> speeds = std::vector<double>(m_iWheelCount);

	for (auto i = 0; i < m_iWheelCount; i++) {
		speeds[i] = velocity * cos((m_vWheels[i].first - direction)* PI / 180.0) + rotate;
	}
	/*
	return cv::Point3d(
		(velocity*cos((150 - direction) * PI / 180.0)) + rotate,
		((velocity*cos((30 - direction)  * PI / 180.0)) + rotate),
		(velocity*cos((270 - direction)  * PI / 180.0)) + rotate
	);
	*/
	return speeds; // make member variable to avoid copy
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

void WheelController::CalculateRobotSpeed()
{
	
	lastSpeed = actualSpeed;
	std::vector<double> _speeds = GetWheelSpeeds();
	cv::Point3d speeds = cv::Point3d(_speeds[0], _speeds[1], _speeds[2]);
	double a, b, c, u, v, w;
	/*
	a = x *[cos(u) * cos(y) + sin(u) * sin(y)] + z
	b = x *[cos(v) * cos(y) + sin(v) * sin(y)] + z
	c = x *[cos(w) * cos(y) + sin(w) * sin(y)] + z
	*/
	if (abs(speeds.z - speeds.x) > 0.0000001) { // c - a == 0
		a = speeds.x; b = speeds.y; c = speeds.z;
		u = deg150; v = deg30; w = deg270;
	}
	else if (abs(speeds.x - speeds.y) > 0.0000001) {
		a = speeds.y; b = speeds.z; c = speeds.x;
		u = deg30; v = deg270; w = deg30;
	}
	else if (abs(speeds.z - speeds.y) > 0.0000001) {
		a = speeds.z; b = speeds.x; c = speeds.y;
		u = deg270; v = deg30; w = deg150;
	}
	else {
		// all equal, rotation only
		actualSpeed.velocity = 0;
		actualSpeed.heading = 0;
		actualSpeed.rotation = speeds.x;
		return;

	}
	double s = (b - a) / (c - a);
	double directionInRad = atan(((cos(v) - cos(u)) - s * (cos(w) - cos(u))) / (s * (sin(w) - sin(u)) - (sin(v) - sin(u))));
	//if (directionInRad < 0) directionInRad += 2 * PI;
	actualSpeed.heading = directionInRad / PI * 180;
	actualSpeed.velocity = (a - c) / ((cos(u) - cos(w)) * cos(directionInRad) + (sin(u) - sin(w)) * sin(directionInRad));
	actualSpeed.rotation = c - (actualSpeed.velocity  * cos(w - directionInRad));

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
	oss << "[WheelController] actual: " << "velocity: " << actualSpeed.velocity << ", heading: " << actualSpeed.heading << ", rotate: " << actualSpeed.rotation << "|";
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
	while (!stop_thread) {
		CalculateRobotSpeed();
#ifdef LIMIT_ACCELERATION
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
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // do not poll serial to fast
	}
	std::cout << "WheelController stoping" << std::endl;
	
}
