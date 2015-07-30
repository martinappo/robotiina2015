#pragma once
#include "types.h"
#include <boost/thread/mutex.hpp>
#include <atomic>
#include "ThreadedClass.h"


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
	FieldState *m_pFieldState;
	ICommunicationModule *m_pCom;
public:
	const std::string name;
	DriveInstruction(const std::string name) : name(name){
	};
	void Init(ICommunicationModule *pCom, FieldState *pFieldState){
		m_pCom = pCom;
		m_pFieldState = pFieldState;
	};
	virtual void onEnter(){
		actionStart = boost::posix_time::microsec_clock::local_time();
	};
	virtual NewDriveMode step(double dt) = 0;
	virtual void onExit(){};

};
class Idle : public DriveInstruction
{
private:
	boost::posix_time::ptime idleStart;
public:
	Idle() : DriveInstruction("IDLE"){};
	virtual void onEnter();
	virtual void onExit();
	virtual NewDriveMode step(double dt);
};

class LocateBall : public DriveInstruction
{
private:
	boost::posix_time::ptime rotateStart;
public:
	LocateBall() : DriveInstruction("LOCATE_BALL"){};
	virtual void onEnter();
	virtual NewDriveMode step(double dt);
};

class LocateHome : public DriveInstruction
{
public:
	LocateHome() : DriveInstruction("LOCATE_HOME"){};
	virtual NewDriveMode step(double dt);
};

class DriveToHome : public DriveInstruction
{
public:
	DriveToHome() : DriveInstruction("DRIVE_TO_HOME"){};
	virtual NewDriveMode step(double dt);
};


class CatchBall : public DriveInstruction
{
private:
	boost::posix_time::ptime catchStart;
public:
	CatchBall() : DriveInstruction("CATCH_BALL"){};
	virtual void onEnter();
	virtual void onExit();
	virtual NewDriveMode step(double dt);
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
	virtual void onEnter();
	virtual NewDriveMode step(double dt);
};


class LocateGate : public DriveInstruction
{
public:
	LocateGate() : DriveInstruction("LOCATE_GATE"){};
	virtual NewDriveMode step(double dt);
};

class AimGate : public DriveInstruction
{
public:
	AimGate() : DriveInstruction("AIM_GATE"){};
	virtual NewDriveMode step(double dt);
};

class Kick : public DriveInstruction
{
public:
	virtual void onEnter();
	Kick() : DriveInstruction("KICK"){};
	virtual NewDriveMode step(double dt);
};

class RecoverCrash : public DriveInstruction
{
public:
	RecoverCrash() : DriveInstruction("RECOVER_CRASH"){};
	virtual NewDriveMode step(double dt);
};

class CoilGun;
class WheelController;

class NewAutoPilot : public IControlModule, public ThreadedClass
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
	ICommunicationModule *m_pComModule;
	FieldState *m_pFieldState;
	/*
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
	*/


	std::atomic_bool drive;
	boost::mutex mutex;
	boost::posix_time::ptime rotateTime = time;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastUpdate = time - boost::posix_time::seconds(60);
	NewDriveMode lastDriveMode = DRIVEMODE_IDLE;
	NewDriveMode driveMode = DRIVEMODE_IDLE;
	NewDriveMode testDriveMode = DRIVEMODE_IDLE;

protected:
	/*
	//	NewDriveMode DriveToBall();
	NewDriveMode LocateBall();
	NewDriveMode CatchBall();
	NewDriveMode LocateGate();
	NewDriveMode LocateHome();
	NewDriveMode DriveToHome();
	NewDriveMode RecoverCrash();
	*/
	void Step();
	void WriteInfoOnScreen();
public:
	NewAutoPilot(ICommunicationModule *pComModule, FieldState *pState);
	//void UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool ballInTribbler, bool sightObstructed, bool somethingOnWay, int borderDistance, cv::Point2i ballCount);
	void setTestMode(NewDriveMode mode);
	void enableTestMode(bool enable);
	void Run();
	virtual ~NewAutoPilot();
	std::string GetDebugInfo();
};
