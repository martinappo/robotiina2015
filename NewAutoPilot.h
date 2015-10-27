#pragma once
#include "types.h"
#include <boost/thread/mutex.hpp>
#include <atomic>
#include "ThreadedClass.h"
#include "FieldState.h"


enum NewDriveMode {
	DRIVEMODE_IDLE = 0,
	DRIVEMODE_LOCATE_BALL,
	DRIVEMODE_DRIVE_TO_BALL,
	DRIVEMODE_LOCATE_HOME,
	DRIVEMODE_DRIVE_HOME,
	DRIVEMODE_CATCH_BALL,
	DRIVEMODE_LOCATE_GATE,
	DRIVEMODE_AIM_GATE,
	DRIVEMODE_KICK,
	DRIVEMODE_RECOVER_CRASH,
	DRIVEMODE_EXIT,
	//2v2 modes
	DRIVEMODE_2V2_OFFENSIVE,
	DRIVEMODE_2V2_DEFENSIVE,
	DRIVEMODE_2V2_KICKOFF,
	DRIVEMODE_2V2_AIM_GATE,
	DRIVEMODE_2V2_KICK,
	DRIVEMODE_2V2_DRIVE_TO_BALL,
	DRIVEMODE_2V2_CATCH_BALL,
	DRIVEMODE_2V2_DRIVE_HOME
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
/*
class LocateBall : public DriveInstruction
{
private:
	boost::posix_time::ptime rotateStart;
public:
	LocateBall() : DriveInstruction("LOCATE_BALL"){};
	virtual void onEnter();
	virtual NewDriveMode step(double dt);
};
*/
class LocateHome : public DriveInstruction
{
public:
	LocateHome() : DriveInstruction("LOCATE_HOME"){};
	virtual NewDriveMode step(double dt);
};

class DriveHome : public DriveInstruction
{
public:
	DriveHome() : DriveInstruction("DRIVE_HOME"){};
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


class LocateGate : public DriveInstruction
{
public:
	LocateGate() : DriveInstruction("LOCATE_GATE"){};
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

class Offensive : public DriveInstruction
{
public:
	Offensive() : DriveInstruction("2V2_AIM_ALLY"){};
	virtual NewDriveMode step(double dt);
};

class Defensive : public DriveInstruction
{
public:
	Defensive() : DriveInstruction("2V2_AIM_ALLY"){};
	virtual NewDriveMode step(double dt);
};

class KickOff : public DriveInstruction
{
private:
	bool active = false;
public:
	KickOff() : DriveInstruction("2V2_KICKOFF"){};
	virtual void onEnter();
	virtual NewDriveMode step(double dt);
};

class AimGate2v2 : public AimGate
{
public:
	AimGate2v2() : AimGate(){};
	virtual NewDriveMode step(double dt);
};

class Kick2v2 : public Kick
{
public:
	Kick2v2() : Kick(){};
	virtual NewDriveMode step(double dt);
};

class DriveToBall2v2 : public DriveInstruction
{
private:
	TargetPosition start;
	BallPosition target;
	double speed;
	double rotate;
	double rotateGate;
	int desiredDistance = 210;
public:
	DriveToBall2v2() : DriveInstruction("2V2_DRIVE_TO_BALL"){};
	virtual void onEnter();
	virtual NewDriveMode step(double dt);
};

class CatchBall2v2 : public DriveInstruction
{
private:
	boost::posix_time::ptime catchStart;
public:
	CatchBall2v2() : DriveInstruction("2V2_CATCH_BALL"){};
	virtual void onEnter();
	virtual NewDriveMode step(double dt);
};

class DriveHome2v2 : public DriveInstruction
{
public:
	DriveHome2v2() : DriveInstruction("2V2_DRIVE_HOME"){};
	virtual void onEnter();
	virtual NewDriveMode step(double dt);
};

