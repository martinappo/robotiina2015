#include "StateMachine.h"

bool DriveInstruction::aimTarget(const ObjectPosition &target, double errorMargin){
	//m_pCom->ToggleTribbler(0);
	double heading = target.getHeading();
	if (abs(heading) > errorMargin){
		std::cout << ", rotating: " << heading;
		m_pCom->Drive(0, 0, -heading);
		return false;
	}
	else{
		m_pCom->Drive(0, 0, 0);
		return true;
	}
}
bool DriveInstruction::catchTarget(const ObjectPosition &target){
	if (m_pCom->BallInTribbler()) {
		m_pCom->Drive(0, 0, 0);
		return true;
	}
	//m_pCom->ToggleTribbler(100);
	m_pCom->Drive(30, 0, target.getHeading());
	return false;
}

bool DriveInstruction::driveToTarget(const ObjectPosition &target, double maxDistance){

	if (USE_ANGLED_DRIVING)
		return driveToTargetWithAngle(target, maxDistance);
	double dist = target.getDistance();
		
	if (dist > maxDistance){
		std::cout << ", ball to far: " << dist << " target: " << maxDistance;
		m_pCom->Drive(std::min(100.0, std::max(20.0, dist)), 0, 0);
		return false;
	}
	else{
		std::cout << ", ball near: " << dist << " target: " << maxDistance;
		m_pCom->Drive(0, 0, 0);
		return true;
	}
}

bool DriveInstruction::driveToTargetWithAngle(const ObjectPosition &target, double maxDistance){
	double heading = target.getHeading();

	double dist = target.getDistance();
	if (dist > maxDistance){
		double speed = (dist > 50) ? 100 : (std::max(dist, 20.0));
		m_pCom->Drive(speed, 0, heading*1.5);//To Do: set speed based on distance
		return false;
	}
	else return true;
}

const BallPosition &DriveInstruction::getClosestBall(){
	return  m_pFieldState->balls.getClosest();
	/*
	int target_distance = INT_MAX;
	int target_index = 0;
	for (int i = 0; i < NUMBER_OF_BALLS; i++) {
		if (abs(m_pFieldState->balls[i].fieldCoords.y) > 250) continue; // too far outside of the field
		if (m_pFieldState->balls[i].getDistance() < target_distance) {
			target_index = i;
			target_distance = m_pFieldState->balls[i].getDistance();
		}
	}
	return m_pFieldState->balls[target_index];
	*/

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
		newMode = curDriveMode->second->step1(double(dt));
		
	auto old = curDriveMode;

	if (testMode) newMode = testDriveMode;

	if (newMode != curDriveMode->first){
		boost::mutex::scoped_lock lock(mutex);
		curDriveMode->second->onExit();
		//m_pCom->Stop();
		curDriveMode = driveModes.find(newMode);
		if (curDriveMode == driveModes.end()) {
			std::cout << "Invalid drive mode from :" << old->second->name << ", reverting to idle" << std::endl;
			curDriveMode = driveModes.find(DRIVEMODE_IDLE);
		}
		std::cout << "state change :" << old->second->name << " ->" << curDriveMode->second->name << std::endl; //TODO debug: miks siia ei j6ua?
		curDriveMode->second->onEnter();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	}
	std::cout << "StateMachine stoping" << std::endl;

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

