#pragma once
#include "types.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

class WheelController;


class RobotTracker
{
private:
	WheelController *wheels;
	boost::atomic<bool> stop_thread;
	boost::thread_group threads;
	cv::Point3d lastSpeed = {0, 0, 0};
	cv::Point3d lastPosition = { 0, 0, 0 }; // distance, direction, heading
	cv::Point3d lastPosHeading = { 0, 0, 0 }; // x,y, heading
public:
	RobotTracker(WheelController *wheels);
	void Run();
	void WriteInfoOnScreen(cv::Point3d acutual_speed, cv::Point3d target_speed, cv::Point3d acceleration, double dt);
	~RobotTracker();
};

