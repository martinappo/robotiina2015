
#include "NewAutoPilot.h"
#include "CoilBoard.h"
#include "WheelController.h"
#include <thread>

std::pair<NewDriveMode, DriveInstruction*> NewDriveModes[] = {
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new Idle()),
//	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_BALL, new LocateBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_HOME, new LocateHome()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_HOME, new DriveHome()),
	//std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_GATE, new LocateGate()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_RECOVER_CRASH, new RecoverCrash()),

	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_OFFENSIVE,		new Offensive()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_DEFENSIVE,		new Defensive()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_KICKOFF,		new KickOff()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_AIM_GATE,		new AimGate2v2()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_KICK,			new Kick2v2()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_TO_BALL,	new DriveToBall2v2()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_CATCH_BALL,	new CatchBall2v2()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_HOME,	new DriveHome2v2()),

	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};


NewAutoPilot::NewAutoPilot(ICommunicationModule *pComModule, FieldState *pState) : driveModes(NewDriveModes, NewDriveModes + sizeof(NewDriveModes) / sizeof(NewDriveModes[0]))
{
	m_pComModule = pComModule;
	m_pFieldState = pState;
	for (auto driveMode : driveModes){
		driveMode.second->Init(pComModule, pState);
	}

	curDriveMode = driveModes.find(DRIVEMODE_IDLE);
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

bool DriveInstruction::aimTarget(ObjectPosition &target, double errorMargin){
	double angle = target.getAngle();
	if (angle > 180)
		angle -= 360;
	if (abs(angle) > errorMargin){
		m_pCom->Drive(0, 0, angle);
		return false;
	}
	else return true;
}

bool DriveInstruction::driveToTarget(ObjectPosition &target, double maxDistance){
	if (USE_ANGLED_DRIVING)
		return driveToTargetWithAngle(target, maxDistance);
	double dist = target.getDistance();
	if (dist > maxDistance){
		m_pCom->Drive((dist > 50) ? 100 : (std::max(dist, 20.0)));//To Do: set speed based on distance
		return false;
	}
	else return true;
}

bool DriveInstruction::driveToTargetWithAngle(ObjectPosition &target, double maxDistance){
	double angle = target.getAngle();
	if (angle > 180)
		angle -= 360;
	double dist = target.getDistance();
	if (dist > maxDistance){
		double speed = (dist > 50) ? 100 : (std::max(dist, 20.0));
		m_pCom->Drive(speed, 0, angle*1.5);//To Do: set speed based on distance
		return false;
	}
	else return true;
}

BallPosition DriveInstruction::getClosestBall(){
	BallPosition target =  m_pFieldState->balls[0];
	target.polarMetricCoords.x = INT_MAX;
	for (BallPosition ball : m_pFieldState->balls) {
		if (abs(ball.fieldCoords.y) > 250) continue;
		if (ball.getDistance() < target.getDistance()) {
			target = ball;
		}
	}
	return target;

}

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
	return false ? DRIVEMODE_DRIVE_HOME : DRIVEMODE_DRIVE_TO_BALL;
}

/*BEGIN LocateHome*/
NewDriveMode LocateHome::step(double dt)
{
	return DRIVEMODE_DRIVE_TO_BALL;
}

/*BEGIN DriveHome*/
NewDriveMode DriveHome::step(double dt){
	ObjectPosition &lastGateLocation = m_pFieldState->GetHomeGate();
	double angle = lastGateLocation.getAngle();
	if (angle > 180)
		angle -= 360;
	if (DriveInstruction::aimTarget(lastGateLocation)){//is facing homeGate?
		if (DriveInstruction::driveToTarget(lastGateLocation)){//is near homeGate?
				if (DriveInstruction::aimTarget(m_pFieldState->GetTargetGate())){//is facing target Gate
				return DRIVEMODE_IDLE;
			}
		}
	}
	return DRIVEMODE_DRIVE_HOME;
}
/*BEGIN DriveToBall*/
void DriveToBall::onEnter()
{
	DriveInstruction::onEnter();
	m_pCom->Drive(0, 0, 0);
	std::chrono::milliseconds dura(200);
	std::this_thread::sleep_for(dura);
	m_pCom->ToggleTribbler(false);

	target = DriveInstruction::getClosestBall();
}

NewDriveMode DriveToBall::step(double dt)
{
	target = DriveInstruction::getClosestBall();
	if (target.polarMetricCoords.x == INT_MAX){ // no valid balls
		return DRIVEMODE_DRIVE_HOME;
	}
	if (target.getDistance() > 10000) return DRIVEMODE_IDLE;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;

	if (DriveInstruction::aimTarget(target)){
		if (DriveInstruction::driveToTarget(target)){
			if (DriveInstruction::aimTarget(target)){
				m_pCom->ToggleTribbler(true);
				return DRIVEMODE_CATCH_BALL;
			}
		}
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
	ObjectPosition &lastBallLocation = m_pFieldState->balls[0];

	//std::cout << catchDuration << std::endl;
	if (m_pCom->BallInTribbler()) {
		return DRIVEMODE_AIM_GATE;
	}
	else if (catchDuration > 2000) { //trying to catch ball for 2 seconds
		return DRIVEMODE_DRIVE_TO_BALL;
	}
	else {
		double rotate = abs(lastBallLocation.getAngle()) * 0.4 + 5;

		//std::cout << "rotate: " << rotate << std::endl;
		//m_pCom->Rotate(lastBallLocation.horizontalAngle < 0, rotate);
		m_pCom->Drive(50, 0, lastBallLocation.getAngle() < 0 ? rotate : -rotate);
		//newAutoPilot.m_pCom->Drive(40, 0, 0);
		//double shake = 10 * sign(sin(1 * (time - actionStart).total_milliseconds()));
		//newAutoPilot.m_pCom->Drive(40, 0, shake);
	}
	return DRIVEMODE_CATCH_BALL;
}
/*END CatchBall*/

/*BEGIN LocateGate*/
/*
NewDriveMode LocateGate::step(double dt)
{
	ObjectPosition &lastGateLocation = m_pFieldState->GetTargetGate();
	bool gateInSight = lastGateLocation.getDistance() > 0;
	bool sightObstructed = m_pFieldState->gateObstructed;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

	if (!m_pCom->BallInTribbler()) return DRIVEMODE_DRIVE_TO_BALL;
	if (gateInSight) {
		std::chrono::milliseconds dura(100); // do we need to sleep?
		std::this_thread::sleep_for(dura);
		m_pCom->Drive(0,0,0);
		return DRIVEMODE_AIM_GATE;
	}
	//m_pCom->Rotate(0, 50);
	
	//int s = sign((int)lastGateLocation.horizontalAngle);

	m_pCom->Drive(0, 0, (lastGateLocation.getAngle() < 0? 1:-1)*30);
	
	return DRIVEMODE_LOCATE_GATE;
}
*/
/*BEGIN AimGate*/
NewDriveMode AimGate::step(double dt)
{

	ObjectPosition &lastGateLocation = m_pFieldState->GetTargetGate();
	bool sightObstructed = m_pFieldState->gateObstructed;
	if (!m_pCom->BallInTribbler()) return DRIVEMODE_DRIVE_TO_BALL;
	if ((boost::posix_time::microsec_clock::local_time() - actionStart).total_milliseconds() > 9000) {
		return DRIVEMODE_KICK;
	}
	if (DriveInstruction::aimTarget(lastGateLocation, 2)){
		if (sightObstructed) { //then move sideways away from gate
			m_pCom->Drive(45, 90, 0);
		}
		else {
			return DRIVEMODE_KICK;
		}
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
	return DRIVEMODE_DRIVE_TO_BALL;

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

	return DRIVEMODE_DRIVE_TO_BALL;
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
		/*
		if (!testMode && ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000)) {
			newMode = DRIVEMODE_IDLE;
		}
		else if (!testMode && somethingOnWay && curDriveMode->first != DRIVEMODE_RECOVER_CRASH && curDriveMode->first != DRIVEMODE_IDLE){
			newMode = DRIVEMODE_RECOVER_CRASH;
		}
		else*/ {
			boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
			boost::posix_time::time_duration::tick_type dt = (time - lastStep).total_milliseconds();
			newMode = curDriveMode->second->step(double(dt));
		}
		auto old = curDriveMode;
		
		if (testMode) newMode = testDriveMode;

		if (newMode != curDriveMode->first){
			boost::mutex::scoped_lock lock(mutex);
			curDriveMode->second->onExit();
			//m_pCom->Stop();
			curDriveMode = driveModes.find(newMode);
			if (curDriveMode == driveModes.end()) {
				std::cout << "Invalid drive mode from :" << old->second->name << ", reverting to drive_to_ball" << std::endl;
				curDriveMode = driveModes.find(DRIVEMODE_DRIVE_TO_BALL);;
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


NewAutoPilot::~NewAutoPilot()
{
	WaitForStop();
	for (auto &mode : driveModes){
		delete mode.second;
	}
	//m_pCom->ToggleTribbler(false);

}

/*2v2 part*/
/*BEGIN Offensive*/
NewDriveMode Offensive::step(double dt){
	if (m_pCom->BallInTribbler()){
		//ATTACK! -> GOAL!!
		return DRIVEMODE_2V2_AIM_GATE;
	}
	else
		return DRIVEMODE_2V2_CATCH_BALL;
	return DRIVEMODE_2V2_KICK;
}

/*BEGIN Defensive*/
NewDriveMode Defensive::step(double dt){
	GatePosition ally;
	if (abs(m_pFieldState->GetHomeGate().getDistance() - ally.getDistance()) > 100){//is ally in defense area?
		if (m_pFieldState->GetHomeGate().getDistance() > 80)
			return DRIVEMODE_2V2_DRIVE_HOME;
		else{
			BallPosition ball = DriveInstruction::getClosestBall();
			if (ball.polarMetricCoords.x != INT_MAX)
				DriveInstruction::aimTarget(ball);
			//block gate & look for ball
		}
	}
	else{
		/*active defense
		positon self between opponent and gate?
		*/
	}
	return DRIVEMODE_2V2_DEFENSIVE;
}

/*BEGIN KickOff*/
void KickOff::onEnter(){
	DriveInstruction::onEnter();
	active = false;
	m_pCom->Drive(0, 0, 0);
	for (BallPosition ball : m_pFieldState->balls) {
		if (ball.getDistance() < 80) {
			active = true;
		}
	}
}
NewDriveMode KickOff::step(double dt){
	if (active){
		if (m_pCom->BallInTribbler()) {
			GatePosition ally;//other robot
			if (DriveInstruction::aimTarget(ally, 5)){
				m_pCom->Kick();
				return DRIVEMODE_2V2_DEFENSIVE;
			}
		}
		else{ // get ball
			BallPosition target = DriveInstruction::getClosestBall();
			if (target.polarMetricCoords.x == INT_MAX || target.getDistance() > 10000){ // no valid balls
				//broke
				//TO DO: something
				return DRIVEMODE_EXIT;
			}
			if (DriveInstruction::aimTarget(target)){
				if (DriveInstruction::driveToTarget(target)){
					if (DriveInstruction::aimTarget(target, 13)){
						m_pCom->ToggleTribbler(true);
						return DRIVEMODE_2V2_KICKOFF;
					}
				}
			}
		}
	}
	else{
		if (m_pCom->BallInTribbler()){
			return DRIVEMODE_2V2_OFFENSIVE;
		}
		else{
			GatePosition ally;//other robot
			if (DriveInstruction::aimTarget(ally, 5)){
				for (BallPosition ball : m_pFieldState->balls) {
					if (ball.getDistance() < 250) { //ball incoming?
						m_pCom->ToggleTribbler(true);//prepare to catch; ride towards ball?
					}
				}
			}
		}
	}
	return DRIVEMODE_2V2_KICKOFF;
}

/*BEGIN AimGate2v2*/
void AimGate2v2::onEnter(){
}
NewDriveMode AimGate2v2::step(double dt){


	ObjectPosition &lastGateLocation = m_pFieldState->GetTargetGate();
	bool sightObstructed = m_pFieldState->gateObstructed;


	if (!m_pCom->BallInTribbler()) return DRIVEMODE_2V2_OFFENSIVE;

	int dir = sign(lastGateLocation.getAngle());
	double angle = lastGateLocation.getAngle();
	if (angle > 180){
		angle -= 360;
	}

	if (abs(angle) < 2) {
		if (sightObstructed) { //then move sideways away from gate
			//std::cout << sightObstructed << std::endl;
			m_pCom->Drive(45, 90, 0);
			std::chrono::milliseconds dura(400); // do we need to sleep?
			std::this_thread::sleep_for(dura);
		}
		else {
			return DRIVEMODE_2V2_KICK;
		}
	}
	else {
		m_pCom->Drive(0, 0, dir*(std::max(cv::norm(angle), 6.0)));
	}
	return DRIVEMODE_2V2_AIM_GATE;
}

/*BEGIN Kick2v2*/
void Kick2v2::onEnter(){
	DriveInstruction::onEnter();
	m_pCom->ToggleTribbler(false);
}

NewDriveMode Kick2v2::step(double dt){
	m_pCom->ToggleTribbler(false);
	m_pCom->Drive(0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	m_pCom->Kick();
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return DRIVEMODE_2V2_DEFENSIVE;
}

/*BEGIN DriveToBall2v2*/
void DriveToBall2v2::onEnter(){
	DriveInstruction::onEnter();
	m_pCom->Drive(0, 0, 0);
	std::chrono::milliseconds dura(200);
	std::this_thread::sleep_for(dura);
	m_pCom->ToggleTribbler(false);

	target = DriveInstruction::getClosestBall();
}

NewDriveMode DriveToBall2v2::step(double dt){
	target = DriveInstruction::getClosestBall();
	if (target.polarMetricCoords.x == INT_MAX){ // no valid balls
		return DRIVEMODE_2V2_DEFENSIVE;
	}
	if (target.getDistance() > 10000) return DRIVEMODE_2V2_DEFENSIVE;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_2V2_AIM_GATE;

	if (DriveInstruction::aimTarget(target)){
		if (DriveInstruction::driveToTarget(target)){
			if (DriveInstruction::aimTarget(target, 13)){
				m_pCom->ToggleTribbler(true);
				return DRIVEMODE_2V2_CATCH_BALL;
			}
		}
	}
	return DRIVEMODE_2V2_DRIVE_TO_BALL;
}

/*BEGIN CatchBall2v2*/
void CatchBall2v2::onEnter(){
	DriveInstruction::onEnter();
	m_pCom->ToggleTribbler(true);
	catchStart = boost::posix_time::microsec_clock::local_time();
	m_pCom->Drive(0, 0, 0);
}

NewDriveMode CatchBall2v2::step(double dt){
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type catchDuration = (time - catchStart).total_milliseconds();
	ObjectPosition &lastBallLocation = m_pFieldState->balls[0];
	if (m_pCom->BallInTribbler()) {
		return DRIVEMODE_2V2_AIM_GATE;
	}
	else if (catchDuration > 2000) { //trying to catch ball for 2 seconds
		return DRIVEMODE_2V2_DRIVE_TO_BALL;
	}
	else {
		double rotate = abs(lastBallLocation.getAngle()) * 0.4 + 5;
		m_pCom->Drive(50, 0, lastBallLocation.getAngle() < 0 ? rotate : -rotate);
	}
	return DRIVEMODE_2V2_CATCH_BALL;
}

/*BEGIN DriveHome2v2*/
void DriveHome2v2::onEnter(){
}

NewDriveMode DriveHome2v2::step(double dt){
	ObjectPosition &lastGateLocation = m_pFieldState->GetHomeGate();
	if (DriveInstruction::aimTarget(lastGateLocation)){
		if (DriveInstruction::driveToTarget(lastGateLocation)){
			if (DriveInstruction::aimTarget(lastGateLocation)){
				return DRIVEMODE_2V2_DEFENSIVE;
			}
		}
	}
	return DRIVEMODE_2V2_DRIVE_HOME;
}