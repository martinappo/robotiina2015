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
class SingleModeIdle : public Idle {

	virtual DriveMode step(double dt) {
		if (m_pFieldState->GAME_MODE_START_SINGLE_PLAY) return DRIVEMODE_DRIVE_TO_BALL;
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

class Offensive : public DriveInstruction
{
public:
	Offensive() : DriveInstruction("2V2_AIM_ALLY"){};
	virtual DriveMode step(double dt);
};

class Defensive : public DriveInstruction
{
public:
	Defensive() : DriveInstruction("2V2_AIM_ALLY"){};
	virtual DriveMode step(double dt);
};

class KickOff : public DriveInstruction
{
private:
	bool active = false;
public:
	KickOff() : DriveInstruction("2V2_KICKOFF"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
};

class AimGate2v2 : public DriveInstruction
{
public:
	AimGate2v2() : DriveInstruction("2V2_AIM_GATE"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
};

class Kick2v2 : public DriveInstruction
{
public:
	Kick2v2() : DriveInstruction("2V2_KICK"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
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
	virtual DriveMode step(double dt);
};

class CatchBall2v2 : public DriveInstruction
{
private:
	boost::posix_time::ptime catchStart;
public:
	CatchBall2v2() : DriveInstruction("2V2_CATCH_BALL"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
};

class DriveHome2v2 : public DriveInstruction
{
public:
	DriveHome2v2() : DriveInstruction("2V2_DRIVE_HOME"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
};

class SingleModePlay : public StateMachine {
public:
	SingleModePlay(ICommunicationModule *pComModule, FieldState *pState);
};