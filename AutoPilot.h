#pragma once
#include "types.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

class CoilGun;
class WheelController;
class Arduino;
enum DriveMode {
	IDLE = 0,
	LOCATE_BALL,
	DRIVE_TO_BALL,
	LOCATE_HOME,
	DRIVE_TO_HOME,
	CATCH_BALL,
	LOCATE_GATE,
	RECOVER_CRASH,
	EXIT
};
class AutoPilot: public IAutoPilot
{
private:
	WheelController *wheels;
	CoilGun *coilgun;
	Arduino *arduino;
	ObjectPosition lastBallLocation;
	ObjectPosition lastGateLocation;
	ObjectPosition lastHomeGateLocation;
	volatile bool ballInSight = false;
	volatile bool gateInSight = false;
	volatile bool homeGateInSight = false;
	volatile bool ballInTribbler = false;
	volatile bool somethingOnWay = false;
	cv::Point3i sonars = { 100, 100, 100 };


	boost::atomic<bool> stop_thread;
	boost::atomic<bool> drive;
	boost::thread_group threads;
	boost::mutex mutex;
	boost::posix_time::ptime rotateTime = time;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastUpdate = time; 
	DriveMode driveMode = IDLE;

protected:
	DriveMode DriveToBall();
	DriveMode LocateBall();
	DriveMode CatchBall();
	DriveMode LocateGate();
	DriveMode LocateHome();
	DriveMode DriveToHome();
	DriveMode RecoverCrash();
	void Step();
	void WriteInfoOnScreen();
public:
	AutoPilot(WheelController *wheels, CoilGun *coilgun, Arduino *arduino);
	void UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool ballInTribbler, bool sightObstructed, bool somethingOnWay, int borderDistance);
	void Run();
	virtual ~AutoPilot();
	std::string GetDebugInfo();
};

