#pragma once

#include "Wheel.h"
#include "types.h"
#include <boost/timer/timer.hpp>
#include "ThreadedClass.h"
#include <atomic>

class WheelController : public ThreadedClass {

private:
	cv::Mat targetSpeedXYW = cv::Mat_<double>(4,1); //x ,y, w (rotation)
	Speed targetSpeed; // velocity, heading, rotation
	Speed actualSpeed; // velocity, heading, rotation.
	Speed lastSpeed;
	cv::Point3d robotPos = { 0, 0, 0 }; // x, y, rotation
	std::vector<int> wheelPositions;
	boost::posix_time::ptime stallTime = boost::posix_time::microsec_clock::local_time() + boost::posix_time::seconds(60);
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
	boost::posix_time::ptime lastUpdate = time;
	std::atomic_bool updateSpeed;
	ISerial *m_pComPort;
	int m_iWheelCount;
protected:
	std::vector<double> CalculateWheelSpeeds(double velocity, double direction, double rotate);
	void CalculateRobotSpeed(); // reverse calc
	std::vector<double> GetWheelSpeeds();
public:
	WheelController(ISerial *port, int iWheelCount = 3);
	void InitDummyWheels();
	void Forward(int speed);
	void rotateBack(int speed);
	bool directControl = false;
    void MoveTo(const CvPoint &);

	void Rotate(bool direction, double speed);
	void Drive(double velocity, double direction = 0, double angularSpeed = 0);
	void DriveRotate(double velocity, double direction, double rotate);
	virtual void Drive(const cv::Point2d &speed, double angularSpeed = 0);
	void Stop();

	const Speed & GetActualSpeed();
	const Speed & GetTargetSpeed();
	bool IsStalled();
	bool HasError();
	virtual ~WheelController();
	void DestroyWheels();
	std::string GetDebugInfo();
	void Run();
	bool IsReal(){
		return m_pComPort != NULL;
	}
	int id_start = 1;

};
