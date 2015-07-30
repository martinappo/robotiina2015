
#include "NewAutoPilot.h"
#include "CoilBoard.h"
#include "WheelController.h"
#include <thread>

std::pair<NewDriveMode, DriveInstruction*> NewDriveModes[] = {
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new Idle()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_BALL, new LocateBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_HOME, new LocateHome()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_HOME, new DriveToHome()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_GATE, new LocateGate()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_RECOVER_CRASH, new RecoverCrash()),

	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};

bool NewAutoPilot::Init(ICommunicationModule *pComModule, FieldState *pState){
	m_pComModule = pComModule;
	m_pFieldState = pState;
	for (auto driveMode : driveModes){
		driveMode.second->Init(pComModule, pState);
	}
	return true;
}

NewAutoPilot::NewAutoPilot(): driveModes(NewDriveModes, NewDriveModes + sizeof(NewDriveModes) / sizeof(NewDriveModes[0]))
{
	curDriveMode = driveModes.find(DRIVEMODE_IDLE);
	stop_thread = false;
	/*
	ballInSight = false;
	gateInSight = false;
	homeGateInSight = false;
	ballInTribbler = false;
	sightObstructed = false;
	somethingOnWay = false;
//	ballCount = 
	lastBallCount = cv::Point2i(0,0);
	*/

}
/*
void NewAutoPilot::UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool ballInTribbler, bool sightObstructed, bool somethingOnWay, int borderDistance, cv::Point2i ballCount)
{
	boost::mutex::scoped_lock lock(mutex);
	ballInSight = ballLocation != NULL;
	gateInSight = gateLocation != NULL;
	if (ballInSight) lastBallLocation = *ballLocation;
	if (gateInSight) lastGateLocation = *gateLocation;
	this->ballInTribbler = ballInTribbler;
	this->sightObstructed = sightObstructed;
	this->somethingOnWay = somethingOnWay;
	this->borderDistance = borderDistance;
	this->ballCount = ballCount;
	if (!testMode) {
		lastUpdate = boost::posix_time::microsec_clock::local_time();
		if (driveMode == DRIVEMODE_IDLE) driveMode = DRIVEMODE_LOCATE_BALL;
	}
}
*/
/*BEGIN Idle*/
void Idle::onEnter()
{
	DriveInstruction::onEnter();
	m_pCom->Drive(0,0,0);
	m_pCom->ToggleTribbler(false);
}

void Idle::onExit()
{
}

NewDriveMode Idle::step(double dt)
{
	//TODO: Figure this out!
	//return (actionStart - newAutoPilot.lastUpdate).total_milliseconds() > 0 ? DRIVEMODE_IDLE : DRIVEMODE_DRIVE_TO_BALL;
	return DRIVEMODE_IDLE;
}

/*BEGIN LocateBall*/
void LocateBall::onEnter()
{
	DriveInstruction::onEnter();
	m_pCom->ToggleTribbler(false);
	rotateStart = boost::posix_time::microsec_clock::local_time();
}

NewDriveMode LocateBall::step(double dt)
{
	auto ballInTribbler = m_pCom->BallInTribbler();
	auto ballInSight = m_pFieldState->balls[0].load().distance >= 0;
	auto lastBallCount = cv::Point(1, 0);//newAutoPilot.lastBallCount;
	
	if (ballInTribbler) return DRIVEMODE_LOCATE_GATE;
	if (ballInSight) return DRIVEMODE_DRIVE_TO_BALL;

	//m_pCom->Stop();
	//return DRIVEMODE_LOCATE_BALL;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateStart).total_milliseconds();
	int dir = lastBallCount.x > lastBallCount.y ? 1 : -1;
 
 		   
	if (rotateDuration < 5700){
	    if(true) {
		float speed = 30*fabs(cos( (float)rotateDuration / 1000));
		//float speed = -0.008*rotateDuration + 50;
		//std::cout << speed <<","<< rotateDuration << std::endl;
		m_pCom->Drive(0,0,dir*speed);
	    
		} else if (rotateDuration < 1000) {
			m_pCom->Drive(0, 0, dir * 50);
		}
		else if (rotateDuration < 2500){
			m_pCom->Drive(0, 0, dir * 35);
		}
		else if (rotateDuration < 4500) {
			m_pCom->Drive(0, 0, dir * 20);
		}
		else if (rotateDuration < 5000) {
			m_pCom->Drive(0, 0, dir * 15);
		}
		else{
			m_pCom->Drive(0,0,0);
		}
		
		return DRIVEMODE_LOCATE_BALL;
	}
	else {
		return DRIVEMODE_DRIVE_TO_HOME;
	}
}
/*BEGIN LocateHome*/
NewDriveMode LocateHome::step(double dt)
{
	return DRIVEMODE_LOCATE_BALL;
}

/*BEGIN DriveToHome*/
NewDriveMode DriveToHome::step(double dt)
{
	m_pCom->Drive(-40,0,0);
	std::chrono::milliseconds dura(300);
	std::this_thread::sleep_for(dura);
	m_pCom->Drive(0,0,0);

	return DRIVEMODE_LOCATE_BALL;
}
/*BEGIN DriveToBall*/
void DriveToBall::onEnter()
{
	DriveInstruction::onEnter();
	//newAutoPilot.lastBallCount = newAutoPilot.ballCount;
	m_pCom->Drive(0, 0, 0);
	std::chrono::milliseconds dura(200);
	std::this_thread::sleep_for(dura);
	m_pCom->ToggleTribbler(false);
	start = m_pFieldState->balls[0].load();
	//Desired distance
	target = { 350, 0, 0 };
}

NewDriveMode DriveToBall::step(double dt)
{
	ObjectPosition lastBallLocation = m_pFieldState->balls[0].load();
	if (lastBallLocation.distance < 0) return DRIVEMODE_LOCATE_BALL;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_LOCATE_GATE;

	//auto &lastBallLocation = newAutoPilot.lastBallLocation;

	if(lastBallLocation.distance < target.distance+50){
		m_pCom->ToggleTribbler(true);
	}
	else{
		m_pCom->ToggleTribbler(false);
	}
	//Ball is close and center
	if ((lastBallLocation.distance < target.distance) && abs(lastBallLocation.horizontalDev) <= 13) {
		m_pCom->ToggleTribbler(true);
		return DRIVEMODE_CATCH_BALL;
	} 
	//Ball is close and not center
	else if (lastBallLocation.distance < target.distance){
		rotate = abs(lastBallLocation.horizontalAngle) * 0.4 + 5;

		//std::cout << "rotate: " << rotate << std::endl;
		//m_pCom->Rotate(lastBallLocation.horizontalAngle < 0, rotate);
		m_pCom->Drive(10, 0, lastBallLocation.horizontalAngle < 0 ? rotate : -rotate);
		m_pCom->ToggleTribbler(true);
	}
	//Ball is far away
	else {
		rotate = abs(lastBallLocation.horizontalAngle) * 0.4 +3;
		//rotate = 0;
		//speed calculation
		if (lastBallLocation.distance > 700){
			speed = 150;
		}
		else{
			speed = lastBallLocation.distance * 0.29 - 94;
		}
		m_pCom->Drive(speed, -lastBallLocation.horizontalAngle, lastBallLocation.horizontalAngle < 0?rotate:-rotate);
	}
	return DRIVEMODE_DRIVE_TO_BALL;
}
/*BEGIN CatchBall*/
void CatchBall::onEnter()
{
	DriveInstruction::onEnter();

	m_pCom->ToggleTribbler(true);
	catchStart = boost::posix_time::microsec_clock::local_time();
	m_pCom->Drive(0, 0, 0);
}
void CatchBall::onExit()
{
	//m_pCom->ToggleTribbler(false);
}
NewDriveMode CatchBall::step(double dt)
{
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type catchDuration = (time - catchStart).total_milliseconds();
	ObjectPosition lastBallLocation = m_pFieldState->balls[0].load();

	//std::cout << catchDuration << std::endl;
	if (m_pCom->BallInTribbler()) {
		return DRIVEMODE_LOCATE_GATE;
	}
	else if (catchDuration > 2000) { //trying to catch ball for 2 seconds
		return DRIVEMODE_LOCATE_BALL;
	}
	else {
		double rotate = abs(lastBallLocation.horizontalAngle) * 0.4 + 5;

		//std::cout << "rotate: " << rotate << std::endl;
		//m_pCom->Rotate(lastBallLocation.horizontalAngle < 0, rotate);
		m_pCom->Drive(50, 0, lastBallLocation.horizontalAngle < 0 ? rotate : -rotate);
		//newAutoPilot.m_pCom->Drive(40, 0, 0);
		//double shake = 10 * sign(sin(1 * (time - actionStart).total_milliseconds()));
		//newAutoPilot.m_pCom->Drive(40, 0, shake);
	}
	return DRIVEMODE_CATCH_BALL;
}
/*END CatchBall*/

/*BEGIN LocateGate*/
NewDriveMode LocateGate::step(double dt)
{
	ObjectPosition lastGateLocation = m_pFieldState->gate;
	bool gateInSight = lastGateLocation.distance > 0;
	bool sightObstructed = m_pFieldState->gateObstructed;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

	if (!m_pCom->BallInTribbler()) return DRIVEMODE_LOCATE_BALL;
	if (gateInSight) {
		std::chrono::milliseconds dura(100); // do we need to sleep?
		std::this_thread::sleep_for(dura);
		m_pCom->Drive(0,0,0);
		return DRIVEMODE_AIM_GATE;
	}
	//m_pCom->Rotate(0, 50);
	
	//int s = sign((int)lastGateLocation.horizontalAngle);

	m_pCom->Drive(0, 0, (lastGateLocation.horizontalAngle < 0? 1:-1)*30);
	
	return DRIVEMODE_LOCATE_GATE;
}
/*BEGIN AimGate*/
NewDriveMode AimGate::step(double dt)
{

	ObjectPosition lastGateLocation = m_pFieldState->gate;
	bool gateInSight = lastGateLocation.distance > 0;
	bool sightObstructed = m_pFieldState->gateObstructed;


	if (!m_pCom->BallInTribbler()) return DRIVEMODE_LOCATE_BALL;
	if (!gateInSight) return DRIVEMODE_LOCATE_GATE;

	if ((boost::posix_time::microsec_clock::local_time() - actionStart).total_milliseconds() > 5000) {
		return DRIVEMODE_KICK;
	}
	int dir = sign(lastGateLocation.horizontalAngle);

	//Turn robot to gate
	if (abs(lastGateLocation.horizontalDev) < 30) {
		if (sightObstructed) { //then move sideways away from gate
			//std::cout << sightObstructed << std::endl;
			m_pCom->Drive(45, 90, 0);
			std::chrono::milliseconds dura(400); // do we need to sleep?
			std::this_thread::sleep_for(dura);
		}
		else {
			return DRIVEMODE_KICK;
		}
	}
	else {
		//rotate calculation for gate
		//int rotate = abs(lastGateLocation.horizontalAngle) * 0.4 + 3; // +3 makes no sense we should aim straight
		int rotate = (abs(lastGateLocation.horizontalAngle) * 0.4 + 6); // should we rotate oposite way?
		m_pCom->Drive(0,0, (lastGateLocation.horizontalAngle < 0 ? 1 : -1) * rotate); 
	}
	return DRIVEMODE_AIM_GATE;

}

/*BEGIN Kick*/
NewDriveMode Kick::step(double dt)
{
	m_pCom->ToggleTribbler(false);
	m_pCom->Drive(0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	m_pCom->Kick();
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return DRIVEMODE_LOCATE_BALL;

}
void Kick::onEnter()
{
	DriveInstruction::onEnter();
	m_pCom->ToggleTribbler(false);
}

/*BEGIN RecoverCrash*/
NewDriveMode RecoverCrash::step(double dt)
{
	double velocity2 = 0, direction2 = 0, rotate2 = 0;
	auto targetSpeed = m_pCom->GetTargetSpeed();

	//Backwards
	m_pCom->Drive(50, 180 - targetSpeed.heading,0);
	std::chrono::milliseconds dura(1000);
	std::this_thread::sleep_for(dura);
	m_pCom->Drive(0, 0, 50);
	std::this_thread::sleep_for(dura);
	
	m_pCom->Drive(0, 0, 0);

	return DRIVEMODE_LOCATE_BALL;
}
void NewAutoPilot::setTestMode(NewDriveMode mode) 
{
	testDriveMode = mode;
}
void NewAutoPilot::enableTestMode(bool enable)
{
	setTestMode(DRIVEMODE_IDLE);
	testMode = enable;
	if(!testMode) m_pComModule->Drive(0,0,0);
}


void NewAutoPilot::Run()
{
	boost::posix_time::ptime lastStep = boost::posix_time::microsec_clock::local_time();
	NewDriveMode newMode = curDriveMode->first;
	curDriveMode->second->onEnter();
	while (!stop_thread){
		if (!testMode && ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000)) {
			newMode = DRIVEMODE_IDLE;
		}
		else if (!testMode /*&& somethingOnWay*/ && curDriveMode->first != DRIVEMODE_RECOVER_CRASH && curDriveMode->first != DRIVEMODE_IDLE){
			newMode = DRIVEMODE_RECOVER_CRASH;
		}
		else {
			boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
			boost::posix_time::time_duration::tick_type dt = (time - lastStep).total_milliseconds();
			newMode = curDriveMode->second->step( dt);
		}
		auto old = curDriveMode;
		
		if (testMode) newMode = testDriveMode;

		if (newMode != curDriveMode->first){
			boost::mutex::scoped_lock lock(mutex);
			curDriveMode->second->onExit();
			//m_pCom->Stop();
			curDriveMode = driveModes.find(newMode);
			if (curDriveMode == driveModes.end()) {
				std::cout << "Invalid drive mode from :" << old->second->name << ", reverting to locate_ball" << std::endl;
				curDriveMode = driveModes.find(DRIVEMODE_LOCATE_BALL);;
			}
			std::cout << "state change :" << old->second->name << " ->" << curDriveMode->second->name << std::endl;

			curDriveMode->second->onEnter();


			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	std::cout << "NewAutoPilot stoping" << std::endl;

}

std::string NewAutoPilot::GetDebugInfo(){
	std::ostringstream oss;
	boost::mutex::scoped_lock lock(mutex);
	oss << "[NewAutoPilot] State: " << curDriveMode->second->name;

	return oss.str();
}

void NewAutoPilot::WriteInfoOnScreen(){
	cv::Mat infoWindow(140, 250, CV_8UC3, cv::Scalar::all(0));
	std::ostringstream oss;
	oss << "State: " << curDriveMode->second->name;
	//std::cout << oss.str() << std::endl;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	/*
	oss << "Ball visible: " << (ballInSight ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	oss << "Gate Visible: " << (gateInSight ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	oss << "Ball in tribbler: " << (ballInTribbler ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 110), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	*/
	cv::imshow("NewAutoPilot", infoWindow);
	cv::waitKey(1);
	return;
}



NewAutoPilot::~NewAutoPilot()
{
	WaitForStop();
	for (auto &mode : driveModes){
		delete mode.second;
	}
	//m_pCom->ToggleTribbler(false);

}
