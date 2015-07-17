
#include "NewAutoPilot.h"
#include "coilBoard.h"
#include "Arduino.h"
#include "wheelcontroller.h"
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


NewAutoPilot::NewAutoPilot(WheelController *wheels, CoilGun *coilgun, Arduino *arduino) :wheels(wheels), coilgun(coilgun), arduino(arduino)
, driveModes(NewDriveModes, NewDriveModes + sizeof(NewDriveModes) / sizeof(NewDriveModes[0]))
{
	curDriveMode = driveModes.find(DRIVEMODE_IDLE);
	stop_thread = false;

	ballInSight = false;
	gateInSight = false;
	homeGateInSight = false;
	ballInTribbler = false;
	sightObstructed = false;
	somethingOnWay = false;
//	ballCount = 
	lastBallCount = cv::Point2i(0,0);

	threads.create_thread(boost::bind(&NewAutoPilot::Run, this));
}

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

/*BEGIN Idle*/
void Idle::onEnter(NewAutoPilot&newAutoPilot)
{
	DriveInstruction::onEnter(newAutoPilot);
	newAutoPilot.wheels->Stop();
	newAutoPilot.coilgun->ToggleTribbler(false);
}

void Idle::onExit(NewAutoPilot& newAutoPilot)
{
}

NewDriveMode Idle::step(NewAutoPilot&newAutoPilot, double dt)
{
	return (actionStart - newAutoPilot.lastUpdate).total_milliseconds() > 0 ? DRIVEMODE_IDLE : DRIVEMODE_DRIVE_TO_BALL;
}

/*BEGIN LocateBall*/
void LocateBall::onEnter(NewAutoPilot&newAutoPilot)
{
	DriveInstruction::onEnter(newAutoPilot);
	newAutoPilot.coilgun->ToggleTribbler(false);
	rotateStart = boost::posix_time::microsec_clock::local_time();
}

NewDriveMode LocateBall::step(NewAutoPilot&newAutoPilot, double dt)
{
	auto &ballInTribbler = newAutoPilot.ballInTribbler;
	auto &ballInSight = newAutoPilot.ballInSight;
	auto &wheels = newAutoPilot.wheels;
	auto &lastBallLocation = newAutoPilot.lastBallLocation;
	auto &lastBallCount = newAutoPilot.lastBallCount;
	
	if (ballInTribbler) return DRIVEMODE_LOCATE_GATE;
	if (ballInSight) return DRIVEMODE_DRIVE_TO_BALL;

	//wheels->Stop();
	//return DRIVEMODE_LOCATE_BALL;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateStart).total_milliseconds();
	int dir = lastBallCount.x > lastBallCount.y;
 
 		   
	if (rotateDuration < 5700){
	    if(true) {
		float speed = 30*fabs(cos( (float)rotateDuration / 1000));
		//float speed = -0.008*rotateDuration + 50;
		//std::cout << speed <<","<< rotateDuration << std::endl;
		wheels->Rotate(dir, speed);
	    
		} else if (rotateDuration < 1000) {
			wheels->Rotate(dir, 50);
		}
		else if (rotateDuration < 2500){
			wheels->Rotate(dir, 35);
		}
		else if (rotateDuration < 4500) {
			wheels->Rotate(dir, 20);
		}
		else if (rotateDuration < 5000) {
			wheels->Rotate(dir, 15);
		}
		else{
			wheels->Stop();
		}
		
		return DRIVEMODE_LOCATE_BALL;
	}
	else {
		return DRIVEMODE_DRIVE_TO_HOME;
	}
}
/*BEGIN LocateHome*/
NewDriveMode LocateHome::step(NewAutoPilot&newAutoPilot, double dt)
{
	return DRIVEMODE_LOCATE_BALL;
}

/*BEGIN DriveToHome*/
NewDriveMode DriveToHome::step(NewAutoPilot&newAutoPilot, double dt)
{
	newAutoPilot.wheels->Forward(-40);
	std::chrono::milliseconds dura(300);
	std::this_thread::sleep_for(dura);
	newAutoPilot.wheels->Forward(0);

	return DRIVEMODE_LOCATE_BALL;
}
/*BEGIN DriveToBall*/
void DriveToBall::onEnter(NewAutoPilot&newAutoPilot)
{
	DriveInstruction::onEnter(newAutoPilot);
	newAutoPilot.lastBallCount = newAutoPilot.ballCount;
	newAutoPilot.wheels->Stop();
	std::chrono::milliseconds dura(200);
	std::this_thread::sleep_for(dura);
	newAutoPilot.coilgun->ToggleTribbler(false);
	start = newAutoPilot.lastBallLocation;
	//Desired distance
	target = { 350, 0, 0 };
}

NewDriveMode DriveToBall::step(NewAutoPilot&newAutoPilot, double dt)
{
	if (!newAutoPilot.ballInSight) return DRIVEMODE_LOCATE_BALL;
	if (newAutoPilot.ballInTribbler) return DRIVEMODE_LOCATE_GATE;

	auto &lastBallLocation = newAutoPilot.lastBallLocation;
	auto &wheels = newAutoPilot.wheels;
	auto &coilgun = newAutoPilot.coilgun;
	if(lastBallLocation.distance < target.distance+50){
		coilgun->ToggleTribbler(true);
	}
	else{
		coilgun->ToggleTribbler(false);
	}
	//Ball is close and center
	if ((lastBallLocation.distance < target.distance) && abs(lastBallLocation.horizontalDev) <= 13) {
		coilgun->ToggleTribbler(true);
		return DRIVEMODE_CATCH_BALL;
	} 
	//Ball is close and not center
	else if (lastBallLocation.distance < target.distance){
		rotate = abs(lastBallLocation.horizontalAngle) * 0.4 + 5;

		//std::cout << "rotate: " << rotate << std::endl;
		//wheels->Rotate(lastBallLocation.horizontalAngle < 0, rotate);
		wheels->DriveRotate(10, 0, lastBallLocation.horizontalAngle < 0 ? rotate : -rotate);
		coilgun->ToggleTribbler(true);
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
		wheels->DriveRotate(speed, -lastBallLocation.horizontalAngle, lastBallLocation.horizontalAngle < 0?rotate:-rotate);
	}
	return DRIVEMODE_DRIVE_TO_BALL;
}
/*BEGIN CatchBall*/
void CatchBall::onEnter(NewAutoPilot&newAutoPilot)
{
	DriveInstruction::onEnter(newAutoPilot);

	newAutoPilot.coilgun->ToggleTribbler(true);
	catchStart = boost::posix_time::microsec_clock::local_time();
	newAutoPilot.wheels->Stop();
}
void CatchBall::onExit(NewAutoPilot& NewAutoPilot)
{
	//newAutoPilot.coilgun->ToggleTribbler(false);
}
NewDriveMode CatchBall::step(NewAutoPilot&newAutoPilot, double dt)
{
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type catchDuration = (time - catchStart).total_milliseconds();
	auto &lastBallLocation = newAutoPilot.lastBallLocation;
	auto &wheels = newAutoPilot.wheels;
	auto &coilgun = newAutoPilot.coilgun;
	//std::cout << catchDuration << std::endl;
	if (newAutoPilot.ballInTribbler) {
		return DRIVEMODE_LOCATE_GATE;
	}
	else if (catchDuration > 2000) { //trying to catch ball for 2 seconds
		return DRIVEMODE_LOCATE_BALL;
	}
	else {
		double rotate = abs(lastBallLocation.horizontalAngle) * 0.4 + 5;

		//std::cout << "rotate: " << rotate << std::endl;
		//wheels->Rotate(lastBallLocation.horizontalAngle < 0, rotate);
		newAutoPilot.wheels->DriveRotate(50, 0, lastBallLocation.horizontalAngle < 0 ? rotate : -rotate);
		//newAutoPilot.wheels->DriveRotate(40, 0, 0);
		//double shake = 10 * sign(sin(1 * (time - actionStart).total_milliseconds()));
		//newAutoPilot.wheels->DriveRotate(40, 0, shake);
	}
	return DRIVEMODE_CATCH_BALL;
}
/*END CatchBall*/

/*BEGIN LocateGate*/
NewDriveMode LocateGate::step(NewAutoPilot&newAutoPilot, double dt)
{
	auto &gateInSight = newAutoPilot.gateInSight;
	auto &coilgun = newAutoPilot.coilgun;
	auto &ballInTribbler = newAutoPilot.ballInTribbler;
	auto &wheels = newAutoPilot.wheels;
	auto &sightObstructed = newAutoPilot.sightObstructed;
	auto &lastGateLocation = newAutoPilot.lastGateLocation;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

	if (!ballInTribbler) return DRIVEMODE_LOCATE_BALL;
	if (gateInSight) {
		std::chrono::milliseconds dura(100); // do we need to sleep?
		std::this_thread::sleep_for(dura);
		wheels->Stop();
		return DRIVEMODE_AIM_GATE;
	}
	//wheels->Rotate(0, 50);
	
	//int s = sign((int)lastGateLocation.horizontalAngle);

	wheels->Rotate(lastGateLocation.horizontalAngle < 0, 30);
	
	return DRIVEMODE_LOCATE_GATE;
}
/*BEGIN AimGate*/
NewDriveMode AimGate::step(NewAutoPilot&newAutoPilot, double dt)
{

	auto &gateInSight = newAutoPilot.gateInSight;
	auto &coilgun = newAutoPilot.coilgun;
	auto &ballInTribbler = newAutoPilot.ballInTribbler;
	auto &wheels = newAutoPilot.wheels;
	auto &sightObstructed = newAutoPilot.sightObstructed;
	auto &lastGateLocation = newAutoPilot.lastGateLocation;


	if (!ballInTribbler) return DRIVEMODE_LOCATE_BALL;
	if (!gateInSight) return DRIVEMODE_LOCATE_GATE;

	if ((boost::posix_time::microsec_clock::local_time() - actionStart).total_milliseconds() > 5000) {
		return DRIVEMODE_KICK;
	}
	int dir = sign(lastGateLocation.horizontalAngle);

	//Turn robot to gate
	if (abs(lastGateLocation.horizontalDev) < 30) {
		if (sightObstructed) { //then move sideways away from gate
			//std::cout << sightObstructed << std::endl;
			wheels->DriveRotate(45, 90, 0);
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
		wheels->Rotate(lastGateLocation.horizontalAngle < 0, rotate); 
	}
	return DRIVEMODE_AIM_GATE;

}

/*BEGIN Kick*/
NewDriveMode Kick::step(NewAutoPilot&newAutoPilot, double dt)
{
	newAutoPilot.coilgun->ToggleTribbler(false);
	newAutoPilot.wheels->Stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	newAutoPilot.coilgun->Kick();
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return DRIVEMODE_LOCATE_BALL;

}
void Kick::onEnter(NewAutoPilot&newAutoPilot)
{
	DriveInstruction::onEnter(newAutoPilot);
	newAutoPilot.coilgun->ToggleTribbler(false);
}

/*BEGIN RecoverCrash*/
NewDriveMode RecoverCrash::step(NewAutoPilot&newAutoPilot, double dt)
{
	double velocity2 = 0, direction2 = 0, rotate2 = 0;
	auto targetSpeed = newAutoPilot.wheels->GetTargetSpeed();

	//Backwards
	newAutoPilot.wheels->Drive(50, 180 - targetSpeed.heading);
	std::chrono::milliseconds dura(1000);
	std::this_thread::sleep_for(dura);
	newAutoPilot.wheels->Rotate(1, 50);
	std::this_thread::sleep_for(dura);
	
	newAutoPilot.wheels->Stop();

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
	if(!testMode) wheels->Stop();
}

void NewAutoPilot::OnFieldStateChanged(const FieldState &state){

}

void NewAutoPilot::Run()
{
	boost::posix_time::ptime lastStep = boost::posix_time::microsec_clock::local_time();
	NewDriveMode newMode = curDriveMode->first;
	curDriveMode->second->onEnter(*this);
	while (!stop_thread){
		if (!testMode && ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000)) {
			newMode = DRIVEMODE_IDLE;
		}
		else if (!testMode && somethingOnWay && curDriveMode->first != DRIVEMODE_RECOVER_CRASH && curDriveMode->first != DRIVEMODE_IDLE){
			newMode = DRIVEMODE_RECOVER_CRASH;
		}
		else {
			boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
			boost::posix_time::time_duration::tick_type dt = (time - lastStep).total_milliseconds();
			newMode = curDriveMode->second->step(*this, dt);
		}
		auto old = curDriveMode;
		
		if (testMode) newMode = testDriveMode;

		if (newMode != curDriveMode->first){
			boost::mutex::scoped_lock lock(mutex);
			curDriveMode->second->onExit(*this);
			//wheels->Stop();
			curDriveMode = driveModes.find(newMode);
			if (curDriveMode == driveModes.end()) {
				std::cout << "Invalid drive mode from :" << old->second->name << ", reverting to locate_ball" << std::endl;
				curDriveMode = driveModes.find(DRIVEMODE_LOCATE_BALL);;
			}
			std::cout << "state change :" << old->second->name << " ->" << curDriveMode->second->name << std::endl;

			curDriveMode->second->onEnter(*this);


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
	cv::imshow("NewAutoPilot", infoWindow);
	cv::waitKey(1);
	return;
}



NewAutoPilot::~NewAutoPilot()
{
	stop_thread = true;
	threads.join_all();
	for (auto &mode : driveModes){
		delete mode.second;
	}
	//coilgun->ToggleTribbler(false);

}
