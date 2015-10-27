#include "StateMachine.h"

StateMachine::StateMachine(ICommunicationModule *pComModule, FieldState *pState, 
	const std::map<DriveMode, DriveInstruction*> &driveModes) :driveModes(driveModes)
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
		newMode = curDriveMode->second->step(double(dt));
		
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
		std::cout << "state change :" << old->second->name << " ->" << curDriveMode->second->name << std::endl;

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
	//m_pCom->ToggleTribbler(false);

}

