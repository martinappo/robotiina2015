#pragma once
#include "types.h"
#include "ThreadedClass.h"
#include "FieldState.h"
typedef int DriveMode;
const DriveMode DRIVEMODE_IDLE = 0;

class DriveInstruction
{
protected:
	boost::posix_time::ptime actionStart;
	FieldState *m_pFieldState;
	ICommunicationModule *m_pCom;
public:
	const std::string name;
	DriveInstruction(const std::string &name) : name(name){
	};
	void Init(ICommunicationModule *pCom, FieldState *pFieldState){
		m_pCom = pCom;
		m_pFieldState = pFieldState;
	};
	virtual void onEnter(){
		actionStart = boost::posix_time::microsec_clock::local_time();
	};
	virtual DriveMode step1(double dt){
		if (m_pFieldState->gameMode == FieldState::GAME_MODE_STOPED){
			m_pCom->Drive(0, 0, 0);
			return DRIVEMODE_IDLE;
		}
		return step(dt);
	};
	virtual DriveMode step(double dt) = 0;
	virtual void onExit(){};

	const static bool USE_ANGLED_DRIVING = true;
	bool aimTarget(const ObjectPosition &target, double errorMargin = (USE_ANGLED_DRIVING) ? 90 : 10);
	bool catchTarget(const ObjectPosition &target);
	bool driveToTarget(const ObjectPosition &target, double maxDistance = 50);
	bool driveToTargetWithAngle(const ObjectPosition &target, double maxDistance = 50);
	const BallPosition &getClosestBall();
};
class Idle : public DriveInstruction
{
private:
	boost::posix_time::ptime idleStart;
public:
	Idle() : DriveInstruction("IDLE"){};
	virtual DriveMode step(double dt){ return DRIVEMODE_IDLE; }
};

class StateMachine : public IControlModule, public ThreadedClass
{
public:
	typedef std::map<DriveMode, DriveInstruction*> TDriveModes;
	const TDriveModes driveModes;
	std::atomic_bool testMode;

private:
	TDriveModes::const_iterator curDriveMode;
	ICommunicationModule *m_pComModule;
	FieldState *m_pFieldState;

	std::atomic_bool drive;
	boost::mutex mutex;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	//boost::posix_time::ptime lastUpdate = time - boost::posix_time::seconds(60);
	DriveMode lastDriveMode = DRIVEMODE_IDLE;
	DriveMode driveMode = DRIVEMODE_IDLE;
	DriveMode testDriveMode = DRIVEMODE_IDLE;

protected:
	void Step();
public:
	StateMachine(ICommunicationModule *pComModule, FieldState *pState, const TDriveModes &driveModes);
	void setTestMode(DriveMode mode);
	void enableTestMode(bool enable);
	void Run();
	virtual ~StateMachine();
	std::string GetDebugInfo();
};
