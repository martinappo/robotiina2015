#include "StateMachine.h"

enum SingleModeDriveStates {
	//DRIVEMODE_IDLE = 0,
	DRIVEMODE_DRIVE_TO_BALL = DRIVEMODE_IDLE + 1,
	DRIVEMODE_LOCATE_HOME,
	DRIVEMODE_DRIVE_HOME,
	DRIVEMODE_CATCH_BALL,
	DRIVEMODE_LOCATE_GATE,
	DRIVEMODE_AIM_GATE,
	DRIVEMODE_KICK,
	DRIVEMODE_RECOVER_CRASH,
	DRIVEMODE_EXIT,

};
class SingleModeIdle : public Idle {

	virtual DriveMode step(double dt) {
		std::string command = m_pCom->GetPlayCommand();
		if (command == "START") return DRIVEMODE_DRIVE_TO_BALL;
		return DRIVEMODE_IDLE;
	}
};

class DriveToBall : public DriveInstruction
{
public:
	DriveToBall() : DriveInstruction("DRIVE_TO_BALL"){};
	virtual DriveMode step(double dt);
};

class CatchBall : public DriveInstruction
{
public:
	CatchBall() : DriveInstruction("CATCH_BALL"){};
	virtual void onEnter();
	virtual void onExit();
	virtual DriveMode step(double dt);
};

class AimGate : public DriveInstruction
{
public:
	AimGate() : DriveInstruction("AIM_GATE"){};
	virtual DriveMode step(double dt);
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