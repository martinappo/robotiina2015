#pragma once
#include "types.h"
#include "ThreadedClass.h"
#include "FieldState.h"
typedef int DriveMode;
const DriveMode DRIVEMODE_CRASH = 0;
const DriveMode DRIVEMODE_BORDER_TO_CLOSE = 1;
const DriveMode DRIVEMODE_IDLE = 2;

class DriveInstruction
{
protected:
	boost::posix_time::ptime actionStart;
	FieldState *m_pFieldState;
	ICommunicationModule *m_pCom;
	Speed speed;
	static DriveMode prevDriveMode;
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
	virtual DriveMode step1(double dt, DriveMode driveMode){
		//not executed in test mode
		if (m_pFieldState->gameMode == FieldState::GAME_MODE_STOPED){
			return DRIVEMODE_IDLE;
		}
/*
		// handle crash
		if (m_pFieldState->collisionWithBorder && driveMode != DRIVEMODE_BORDER_TO_CLOSE){
			prevDriveMode = driveMode;
			std::cout << "To close to border" << std::endl;
			return DRIVEMODE_BORDER_TO_CLOSE;
		}
*/
		if (m_pFieldState->collisionWithUnknown && driveMode != DRIVEMODE_CRASH){
			std::cout << "Crash" << std::endl;
			return DRIVEMODE_CRASH;
		}
		// recover from crash
/*
		if (!m_pFieldState->collisionWithBorder && driveMode == DRIVEMODE_BORDER_TO_CLOSE){
			DriveMode tmp = prevDriveMode;
			prevDriveMode = DRIVEMODE_IDLE;
			//std::cout <<"aaa" <<std::endl;
			return tmp;
		}
*/
		if (!m_pFieldState->collisionWithUnknown && driveMode == DRIVEMODE_CRASH){
			DriveMode tmp = prevDriveMode;
			prevDriveMode = DRIVEMODE_IDLE;
			//std::cout << "bbb" << std::endl;
			return tmp;
		}

		prevDriveMode = driveMode;
		speed = { 0, 0, 0 };
		return step(dt);
	};
	virtual DriveMode step(double dt) = 0;
	virtual void onExit(){};

	const static bool USE_ANGLED_DRIVING = false;
	bool aimTarget(const ObjectPosition &target, Speed &speed, double errorMargin = (USE_ANGLED_DRIVING) ? 90 : 10);
	bool catchTarget(const ObjectPosition &target, Speed &speed);
	bool driveToTarget(const ObjectPosition &target, Speed &speed, double maxDistance = 50);
	bool driveToTargetWithAngle(const ObjectPosition &target, Speed &speed, double maxDistance = 50, double errorMargin = 10);
	const BallPosition &getClosestBall(bool includeHeading = false, bool reset=true);
};
class Idle : public DriveInstruction
{
private:
	boost::posix_time::ptime idleStart;
public:
	Idle() : DriveInstruction("IDLE"){};
	void onEnter(){
		m_pCom->Drive(0,0,0);
	}
	virtual DriveMode step(double dt){ return DRIVEMODE_IDLE; }
};

class Crash : public DriveInstruction
{
private:
	boost::posix_time::ptime idleStart;
public:
	Crash() : DriveInstruction("CRASH"){};
	void onEnter(){
//		m_pCom->Kick(2700);
		m_pCom->Drive(0, 0, 0);
		//Sleep(1000);
	}
	virtual DriveMode step(double dt){ 
		return DRIVEMODE_CRASH;
	}
};

class BorderToClose : public DriveInstruction
{
private:
	boost::posix_time::ptime idleStart;
public:
	BorderToClose() : DriveInstruction("BORDER_TO_CLOSE"){};
	void onEnter(){
		m_pCom->Drive(0, 0, 0);
	}
	virtual DriveMode step(double dt){ 
		return DRIVEMODE_BORDER_TO_CLOSE; 
	}
};

class StateMachine : public IControlModule, public ThreadedClass
{
public:
	typedef std::map<DriveMode, DriveInstruction*> TDriveModes;
	const TDriveModes driveModes;
	std::atomic_bool testMode;
	DriveMode preCrashState = DRIVEMODE_IDLE;
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

