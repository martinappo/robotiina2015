#pragma once
#include "types.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <atomic>

enum NewDriveMode {
	DRIVEMODE_IDLE = 0,
	DRIVEMODE_LOCATE_BALL,
	DRIVEMODE_DRIVE_TO_BALL,
	DRIVEMODE_LOCATE_HOME,
	DRIVEMODE_DRIVE_TO_HOME,
	DRIVEMODE_CATCH_BALL,
	DRIVEMODE_LOCATE_GATE,
	DRIVEMODE_AIM_GATE,
	DRIVEMODE_KICK,
	DRIVEMODE_RECOVER_CRASH,
	DRIVEMODE_EXIT
};
class NewAutoPilot;

class DriveInstruction
{
protected:
	boost::posix_time::ptime actionStart;
private:
public:
	const std::string name;
	DriveInstruction(const std::string name) : name(name){};
	virtual void onEnter(NewAutoPilot&NewAutoPilot){
		actionStart = boost::posix_time::microsec_clock::local_time();
	};
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt) = 0;
	virtual void onExit(NewAutoPilot& NewAutoPilot){};

};
class Idle : public DriveInstruction
{
private:
	boost::posix_time::ptime idleStart;
public:
	Idle() : DriveInstruction("IDLE"){};
	virtual void onEnter(NewAutoPilot&NewAutoPilot);
	virtual void onExit(NewAutoPilot& NewAutoPilot);
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};

class LocateBall : public DriveInstruction
{
private:
	boost::posix_time::ptime rotateStart;
public:
	LocateBall() : DriveInstruction("LOCATE_BALL"){};
	virtual void onEnter(NewAutoPilot&NewAutoPilot);
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};

class LocateHome : public DriveInstruction
{
public:
	LocateHome() : DriveInstruction("LOCATE_HOME"){};
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};

class DriveToHome : public DriveInstruction
{
public:
	DriveToHome() : DriveInstruction("DRIVE_TO_HOME"){};
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};


class CatchBall : public DriveInstruction
{
private:
	boost::posix_time::ptime catchStart;
public:
	CatchBall() : DriveInstruction("CATCH_BALL"){};
	virtual void onEnter(NewAutoPilot&NewAutoPilot);
	virtual void onExit(NewAutoPilot& NewAutoPilot);
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};

class DriveToBall : public DriveInstruction
{
private:
	ObjectPosition start;
	ObjectPosition target;
	double speed;
	double rotate;
	double rotateGate;
	int desiredDistance = 210;
public:
	DriveToBall() : DriveInstruction("DRIVE_TO_BALL"){};
	virtual void onEnter(NewAutoPilot&NewAutoPilot);
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};


class LocateGate : public DriveInstruction
{
public:
	LocateGate() : DriveInstruction("LOCATE_GATE"){};
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};

class AimGate : public DriveInstruction
{
public:
	AimGate() : DriveInstruction("AIM_GATE"){};
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};

class Kick : public DriveInstruction
{
public:
	virtual void onEnter(NewAutoPilot&NewAutoPilot);
	Kick() : DriveInstruction("KICK"){};
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};

class RecoverCrash : public DriveInstruction
{
public:
	RecoverCrash() : DriveInstruction("RECOVER_CRASH"){};
	virtual NewDriveMode step(NewAutoPilot&NewAutoPilot, double dt);
};

class CoilGun;
class WheelController;

class NewAutoPilot : public IFieldStateListener
{
	friend class Idle;
	friend class DriveToBall;
	friend class CatchBall;
	friend class LocateBall;
	friend class LocateHome;
	friend class DriveToHome;
	friend class LocateGate;
	friend class AimGate;
	friend class Kick;
	friend class RecoverCrash;
public:
	std::map<NewDriveMode, DriveInstruction*> driveModes;
	std::atomic_bool testMode;

private:
	std::map<NewDriveMode, DriveInstruction*>::iterator curDriveMode;
	WheelController *wheels;
	CoilGun *coilgun;
	ObjectPosition lastBallLocation;
	ObjectPosition lastGateLocation;
	ObjectPosition lastHomeGateLocation;

	std::atomic_bool ballInSight;
	std::atomic_bool gateInSight;
	std::atomic_bool homeGateInSight;
	std::atomic_bool ballInTribbler;
	std::atomic_bool sightObstructed;
	std::atomic_bool somethingOnWay;
	std::atomic_int borderDistance;
	boost::atomic<cv::Point2i> ballCount;
	cv::Point2i lastBallCount;



	std::atomic_bool stop_thread;
	std::atomic_bool drive;
	boost::thread_group threads;
	boost::mutex mutex;
	boost::posix_time::ptime rotateTime = time;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastUpdate = time - boost::posix_time::seconds(60);
	NewDriveMode lastDriveMode = DRIVEMODE_IDLE;
	NewDriveMode driveMode = DRIVEMODE_IDLE;
	NewDriveMode testDriveMode = DRIVEMODE_IDLE;

protected:
	//	NewDriveMode DriveToBall();
	NewDriveMode LocateBall();
	NewDriveMode CatchBall();
	NewDriveMode LocateGate();
	NewDriveMode LocateHome();
	NewDriveMode DriveToHome();
	NewDriveMode RecoverCrash();
	void Step();
	void WriteInfoOnScreen();
public:
	NewAutoPilot(WheelController *wheels, CoilGun *coilgun);
	virtual void OnFieldStateChanged(const FieldState &state);
	void UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool ballInTribbler, bool sightObstructed, bool somethingOnWay, int borderDistance, cv::Point2i ballCount);
	void setTestMode(NewDriveMode mode);
	void enableTestMode(bool enable);
	void Run();
	virtual ~NewAutoPilot();
	std::string GetDebugInfo();
};
