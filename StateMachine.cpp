#include "StateMachine.h"
#include <algorithm>

bool DriveInstruction::aimTarget(const ObjectPosition &target, Speed &speed, double errorMargin){
	double heading = target.getHeading();
	if (fabs(heading) > errorMargin){
		speed.rotation = - sign0(heading) * std::min(40.0, std::max(fabs(heading), 5.0));
		return false;
	}
	else{
		return fabs(heading) < errorMargin;
	}
}
bool DriveInstruction::catchTarget(const ObjectPosition &target, Speed &speed){
	if (m_pCom->BallInTribbler()) {
		return true;
	}
	double heading =  target.getHeading();
	double dist = target.getDistance();
	//m_pCom->Drive(50, 0, -sign(heading)* 5);
	speed.velocity = 50;
	speed.heading = -sign0(heading) * 5;
	return false;
}

bool DriveInstruction::driveToTarget(const ObjectPosition &target, Speed &speed, double maxDistance){
	double dist = target.getDistance();
		
	if (dist > maxDistance){
		//std::cout << ", ball to far: " << dist << " target: " << maxDistance;
		//m_pCom->Drive(std::max(20.0, dist), 0, 0);
		speed.velocity = std::max(20.0, dist);
		return false;
	}
	else{
		//std::cout << ", ball near: " << dist << " target: " << maxDistance;
		//m_pCom->Drive(20, 0, 0);
		speed.velocity = 20;
		return true;
	}
}

bool DriveInstruction::driveToTargetWithAngle(const ObjectPosition &target, Speed &speed, double maxDistance, double errorMargin){
	double heading = target.getHeading();
	double dist = target.getDistance();
	double velocity = 0;
	double direction = 0;
	double angularSpeed = 0;
	bool onPoint = false;
	if (fabs(heading) > errorMargin){
		if (dist > maxDistance){
			velocity = std::max(30.0, dist); //max speed is limited to 190 in wheelController.cpp 
			direction = heading; //drive towards target
			angularSpeed = sign0(heading) * 20; //meanwhile rotate slowly to face the target
		}
		else{ //at location but facing wrong way
			angularSpeed = sign0(heading) * std::max(fabs(heading) * 0.5, 10.0); //rotate
		}
	}
	else{
		if (dist > maxDistance){
			velocity = std::max(30.0, dist);
			direction = heading; //drive towards target
		}
		else onPoint = true;
	}
	//m_pCom->Drive(speed, direction, -angularSpeed); 
	speed.velocity = velocity;
	speed.heading = direction;
	speed.rotation = -angularSpeed;
	return onPoint;
}


const BallPosition &DriveInstruction::getClosestBall(bool includeHeading, bool reset){
	return  m_pFieldState->balls.getClosest();
}


StateMachine::StateMachine(ICommunicationModule *pComModule, FieldState *pState, 
	const std::map<DriveMode, DriveInstruction*> &driveModes) :driveModes(driveModes), ThreadedClass("Autopilot/StateMachine")
{
	m_pComModule = pComModule;
	m_pFieldState = pState;
	for (auto driveMode : driveModes){
		driveMode.second->Init(pComModule, pState);
	}

	curDriveMode = this->driveModes.find(DRIVEMODE_IDLE);
}

void StateMachine::setTestMode(DriveMode mode)
{
	testDriveMode = mode;
}
void StateMachine::enableTestMode(bool enable)
{
	setTestMode(DRIVEMODE_IDLE);
	testMode = enable;
	if (!testMode) m_pComModule->Drive(0, 0, 0);
}


void StateMachine::Run()
{
	boost::posix_time::ptime lastStep = boost::posix_time::microsec_clock::local_time();
	DriveMode newMode = curDriveMode->first;
	curDriveMode->second->onEnter();
	while (!stop_thread) {
		boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type dt = (time - lastStep).total_milliseconds();
		newMode = testMode ? curDriveMode->second->step(double(dt)) : curDriveMode->second->step1(double(dt));
		
	auto old = curDriveMode;

	if (testMode){
		if (testDriveMode != DRIVEMODE_IDLE && newMode == DRIVEMODE_IDLE){
			newMode = testDriveMode;
		}
		else if (newMode != testDriveMode) {
			newMode = DRIVEMODE_IDLE;
			testDriveMode = DRIVEMODE_IDLE;
		}
	}

	if (newMode != curDriveMode->first){
		boost::mutex::scoped_lock lock(mutex);
		curDriveMode->second->onExit();
		//m_pCom->Stop();
		curDriveMode = driveModes.find(newMode);
		if (curDriveMode == driveModes.end()) {
			//std::cout << "Invalid drive mode from :" << old->second->name << ", reverting to idle" << std::endl;
			curDriveMode = driveModes.find(DRIVEMODE_IDLE);
		}
		//std::cout << "state change :" << old->second->name << " ->" << curDriveMode->second->name << std::endl; //TODO debug: miks siia ei j6ua?
		curDriveMode->second->onEnter();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	}
	//std::cout << "StateMachine stoping" << std::endl;

}

std::string StateMachine::GetDebugInfo(){
	std::ostringstream oss;
	boost::mutex::scoped_lock lock(mutex);
	oss << "[StateMachine] State: " << curDriveMode->second->name;

	return oss.str();
}


StateMachine::~StateMachine()
{
	WaitForStop();
	for (auto &mode : driveModes){
		delete mode.second;
	}
}

