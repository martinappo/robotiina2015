#include <thread>
#include "RobotTracker.h"
#include "WheelController.h"

RobotTracker::RobotTracker(WheelController *wheels) :wheels(wheels)
{
	threads.create_thread(boost::bind(&RobotTracker::Run, this));

}

void RobotTracker::Run()
{
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
	double velocity = 0, direction = 0, rotate = 0;
	double velocity2 = 0, direction2 = 0, rotate2 = 0;

	while (!stop_thread) {
		/*
		time = boost::posix_time::microsec_clock::local_time();
		double dt = (time - lastStep).total_milliseconds();
		wheels->GetRobotSpeed(velocity, direction, rotate);
		wheels->GetTargetSpeed(velocity2, direction2, rotate2);

		cv::Point3d acceleration = { (velocity - lastSpeed.x) / dt, (direction - lastSpeed.y) / dt, (rotate - lastSpeed.z) / dt };
		cv::Point3d current_speed = { velocity, direction, rotate };
		cv::Point3d target_speed = { velocity2, direction2, rotate2 };
		cv::Point3d position;

		position.x = lastPosition.x + velocity * dt + 0.5 * (acceleration.x) * pow(dt, 2);
		*/
		/*
		positoion.y = lastPosition.x
		// from polar coordinates to ...
		lastSpeed.x = sin(direction* PI / 180.0)* velocity + rotate;
		lastSpeed.y = cos(direction* PI / 180.0)* velocity + rotate,
		lastSpeed.z = rotate;
		*/
		lastStep = time;
		//lastSpeed = current_speed;

		// speed update interval is 62.5Hz
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // do not poll serial to fast

	}
}


RobotTracker::~RobotTracker()
{
	stop_thread = true;
	threads.join_all();
}

