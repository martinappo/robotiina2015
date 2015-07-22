#pragma once

#include "Wheel.h"
#include "types.h"
#include <boost/timer/timer.hpp>
#include "ThreadedClass.h"
#include <atomic>

class WheelController: public IWheelController,ThreadedClass {
private:
	Speed targetSpeed; // velocity, heading, rotation
	Speed actualSpeed; // velocity, heading, rotation.
	Speed lastSpeed;
	cv::Point3d robotPos = { 0, 0, 0 }; // x, y, rotation
	BasicWheel * w_left;
	BasicWheel * w_right;
	BasicWheel * w_back;
	boost::posix_time::ptime stallTime = boost::posix_time::microsec_clock::local_time() + boost::posix_time::seconds(60);
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
	boost::posix_time::ptime lastUpdate = time;
	std::atomic_bool updateSpeed;
protected:
	cv::Point3d CalculateWheelSpeeds(double velocity, double direction, double rotate);
	void CalculateRobotSpeed(); // reverse calc
	cv::Point3d GetWheelSpeeds();
public:
	WheelController();
	void InitWheels(boost::asio::io_service &io);
	void InitDummyWheels();
	void Forward(int speed);
	void rotateBack(int speed);
	bool directControl = false;
    void MoveTo(const CvPoint &);

	void Rotate(bool direction, double speed);
	void Drive(double velocity, double direction, double rotate);
	void DriveRotate(double velocity, double direction, double rotate);
	void Stop();

	const Speed & GetActualSpeed();
	const Speed & GetTargetSpeed();
	bool IsStalled();
	bool HasError();
	~WheelController();
	void DestroyWheels();
	std::string GetDebugInfo();
	void Run();


};
