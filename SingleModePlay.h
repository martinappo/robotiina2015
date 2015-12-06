#include "StateMachine.h"

enum SingleModeDriveStates {
	//DRIVEMODE_IDLE = 0,
	DRIVEMODE_DRIVE_TO_BALL = DRIVEMODE_IDLE + 1,
	DRIVEMODE_LOCATE_HOME,
	DRIVEMODE_DRIVE_HOME,
	DRIVEMODE_DRIVE_HOME_AT_START,
	DRIVEMODE_CATCH_BALL,
	DRIVEMODE_LOCATE_GATE,
	DRIVEMODE_AIM_GATE,
	DRIVEMODE_KICK,
	DRIVEMODE_RECOVER_CRASH,
	DRIVEMODE_EXIT,

};
class SingleModeIdle : public Idle {
	virtual void onEnter(){
		m_pCom->ToggleTribbler(0);
		m_pCom->Drive(0, 0, 0);
	}

	virtual DriveMode step(double dt) {
		if (!m_pFieldState->isPlaying) {
			return DRIVEMODE_IDLE;
		}
		return DRIVEMODE_DRIVE_HOME_AT_START;
	}
};

class DriveToBall : public DriveInstruction
{
public:
	virtual void onEnter();
	DriveToBall(const std::string &name = "DRIVE_TO_BALL") : DriveInstruction(name){};
	virtual DriveMode step(double dt);
	/*
protected:
	virtual DriveMode stepNaive(double dt);
	virtual DriveMode stepAngled(double dt);
	virtual DriveMode stepPenatalizeRotation(double dt);
	virtual DriveMode stepAimGate(double dt);
	*/

private:
	bool toggledDribbler = false;
	ObjectPosition initialBall;
	ObjectPosition initialGate;
};

class CatchBall : public DriveInstruction
{
public:
	CatchBall(const std::string &name = "CATCH_BALL") : DriveInstruction(name){};
	virtual void onEnter();
	virtual void onExit();
	virtual DriveMode step(double dt);
protected:
	double initDist;
};

class AimGate : public DriveInstruction
{
public:
	AimGate() : DriveInstruction("AIM_GATE"){};
	virtual DriveMode step(double dt);
protected:
	boost::posix_time::ptime dodgeTime = boost::posix_time::microsec_clock::local_time();
	bool alreadyDodged = false;
	Speed lastSpeed;
};

class Kick : public DriveInstruction
{
public:
	virtual void onEnter();
	Kick() : DriveInstruction("KICK"){};
	virtual DriveMode step(double dt);
};


class SingleModePlay : public StateMachine {
public:
	SingleModePlay(ICommunicationModule *pComModule, FieldState *pState);
};