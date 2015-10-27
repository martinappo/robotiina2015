
#include "NewAutoPilot.h"
#include "CoilBoard.h"
#include "WheelController.h"
#include <thread>

std::pair<NewDriveMode, DriveInstruction*> SingleDriveModes[] = {
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
};
std::pair<NewDriveMode, DriveInstruction*> DoubleDriveModes[] = {
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_OFFENSIVE, new Offensive()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_DEFENSIVE,		new Defensive()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_KICKOFF,		new KickOff()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_AIM_GATE,		new AimGate2v2()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_KICK,			new Kick2v2()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_TO_BALL,	new DriveToBall2v2()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_CATCH_BALL,	new CatchBall2v2()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_HOME,	new DriveHome2v2()),

	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};


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
NewDriveMode DriveHome::step(double dt)
{

	/*double move1 = 100 * sin(dt / 4000);
	double move2 = 360 * cos(dt / 4000);
	m_pCom->Drive(move1, move2, -move1);*/

	ObjectPosition &lastGateLocation = m_pFieldState->GetHomeGate();
	double angle = lastGateLocation.getAngle();
	if (angle > 180)
		angle -= 360;
	if (abs(angle) < 10){
		if (lastGateLocation.getDistance() > 10 ){
			m_pCom->Drive(90);
		}
		else{
			double angle = m_pFieldState->GetTargetGate().getAngle();
			if (angle > 180)
				angle -= 360;
			if (abs(angle) > 5){
				m_pCom->Drive(0, 0, angle);
				return DRIVEMODE_IDLE;
			}
		}
	}
	else {
		m_pCom->Drive(0, 0, angle);
	}

	/*
	m_pCom->Drive(-40,0,0);
	std::chrono::milliseconds dura(300);
	std::this_thread::sleep_for(dura);
	m_pCom->Drive(0,0,0);
	*/
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

	target = m_pFieldState->balls[0];
	for (BallPosition ball : m_pFieldState->balls) {
		if (ball.getDistance() < target.getDistance()) {
			target = ball;
		}
	}
}

NewDriveMode DriveToBall::step(double dt)
{
	target.polarMetricCoords.x = INT_MAX;
	for (BallPosition ball : m_pFieldState->balls) {
		if (abs(ball.fieldCoords.y) > 250) continue; // too far outside of the field
		//if (abs(ball.fieldCoords.y) > 250) continue;
		if (ball.getDistance() < target.getDistance()) {
			target = ball;
		}
	}
	std::cout << target.fieldCoords << target.getDistance() << "  " << target.getAngle() << std::endl;
	if (target.polarMetricCoords.x == INT_MAX){ // no valid balls
		return DRIVEMODE_DRIVE_HOME;
	}
	//ObjectPosition &lastBallLocation = m_pFieldState->balls[0];
	if (target.getDistance() > 10000) return DRIVEMODE_IDLE;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;


	//Ball is close
	double angle = target.getAngle();
	if (angle > 180)
		angle = angle - 360;
	if (target.getDistance() < 20) {
		// Ball is centered
		if (abs(angle) <= 13) {
			m_pCom->ToggleTribbler(true);
			return DRIVEMODE_CATCH_BALL;
		}
		// Ball is not centered
		else {
			m_pCom->Drive(0, 0, angle);
			m_pCom->ToggleTribbler(true);
		}
		
	} 
	//Ball is far away
	else {
		//rotate = 0;
		//speed calculation
		//(cv::norm(angle) > 12){
			//m_pCom->Drive(0, 0, 60*sign(angle));
			
		//}
		//else{
			if (target.getDistance() > 100){
				speed = 60;
			}
			else{
				speed = target.getDistance(); // TODO: ilmselt veidi väiksemaks
			}
			double angleConst = abs(angle) < 13? 0 : std::max(std::min(50.0 / target.getDistance(), 1.0), 0.5);
			std::cout << angleConst << std::endl;
			m_pCom->Drive(speed, angle, angleConst * angle); // TODO: mingi väikese kaarega sõita
		//}
	}
	return DRIVEMODE_DRIVE_TO_BALL;
}

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
	//bool gateInSight = lastGateLocation.getDistance() > 0;
	bool sightObstructed = m_pFieldState->gateObstructed;


	if (!m_pCom->BallInTribbler()) return DRIVEMODE_DRIVE_TO_BALL;
	//if (!gateInSight) return DRIVEMODE_LOCATE_GATE;

	if ((boost::posix_time::microsec_clock::local_time() - actionStart).total_milliseconds() > 9000) {
		return DRIVEMODE_KICK;
	}
	int dir = sign(lastGateLocation.getAngle());
	double angle = lastGateLocation.getAngle();
	if (angle > 180){
		angle -= 360;
	}

	//Turn robot to gate
	if (abs(angle) < 2) {
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
		//int rotate = (int)((abs(lastGateLocation.getAngle()) * 0.4 + 6)); // should we rotate oposite way?
		m_pCom->Drive(0,0, dir*(std::max(cv::norm(angle), 6.0)));
	}
	return DRIVEMODE_AIM_GATE;

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
	if (abs(m_pFieldState->GetHomeGate().getDistance() - ally.getDistance()) > 100){
		if (m_pFieldState->GetHomeGate().getDistance() > 80)
			return DRIVEMODE_2V2_DRIVE_HOME;
		else{
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
			if (abs(ally.getAngle()) < 5){
				m_pCom->Kick();
				return DRIVEMODE_2V2_DEFENSIVE;
			}
			else{
				//aim ally
			}
		}
		else{ // get ball
			BallPosition target;
			target.polarMetricCoords.x = INT_MAX;
			for (BallPosition ball : m_pFieldState->balls) {
				if (abs(ball.fieldCoords.y) > 250) continue; // too far outside of the field
				if (ball.getDistance() < target.getDistance()) {
					target = ball;
				}
			}
			if (target.polarMetricCoords.x == INT_MAX || target.getDistance() > 10000){ // no valid balls
				//broke
				//TO DO: something
				return DRIVEMODE_EXIT;
			}
			//Ball is close
			double angle = target.getAngle();
			if (angle > 180)
				angle = angle - 360;
			//ball is close enough for catching
			if (target.getDistance() < 20) {
				// Ball is centered
				if (abs(angle) <= 13) {
					m_pCom->ToggleTribbler(true);
				}
				// Ball is not centered
				else {
					m_pCom->Drive(0, 0, angle);
					m_pCom->ToggleTribbler(true);
				}
			}
			// ball is still to far -  go closer
			else {
				double angleConst = abs(angle) < 13 ? 0 : std::max(std::min(50.0 / target.getDistance(), 1.0), 0.5);
				m_pCom->Drive(target.getDistance(), angle, angleConst * angle);
			}
		}
	}
	else{
		if (m_pCom->BallInTribbler()){
			return DRIVEMODE_2V2_OFFENSIVE;
		}
		else{
			GatePosition ally;//other robot
			if (abs(ally.getAngle()) < 5){ //facing other robot?
				for (BallPosition ball : m_pFieldState->balls) { 
					if (ball.getDistance() < 250) { //ball incoming?
						m_pCom->ToggleTribbler(true);//prepare to catch; ride towards ball?
					}
				}
			}
			else{
				//aim ally
			}
		}
	}
	return DRIVEMODE_2V2_KICKOFF;
}

/*BEGIN AimGate2v2*/
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
NewDriveMode Kick2v2::step(double dt){
	__super::step(dt);
	return DRIVEMODE_2V2_DEFENSIVE;
}

/*BEGIN DriveToBall2v2*/
void DriveToBall2v2::onEnter(){
	DriveInstruction::onEnter();
	m_pCom->Drive(0, 0, 0);
	std::chrono::milliseconds dura(200);
	std::this_thread::sleep_for(dura);
	m_pCom->ToggleTribbler(false);

	target = m_pFieldState->balls[0];
	for (BallPosition ball : m_pFieldState->balls) {
		if (ball.getDistance() < target.getDistance()) {
			target = ball;
		}
	}
}

NewDriveMode DriveToBall2v2::step(double dt){
	target.polarMetricCoords.x = INT_MAX;
	for (BallPosition ball : m_pFieldState->balls) {
		if (abs(ball.fieldCoords.y) > 250) continue; 
		if (ball.getDistance() < target.getDistance()) {
			target = ball;
		}
	}
	if (target.polarMetricCoords.x == INT_MAX){ // no valid balls
		return DRIVEMODE_2V2_DEFENSIVE;
	}
	if (target.getDistance() > 10000) return DRIVEMODE_2V2_DEFENSIVE;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_2V2_AIM_GATE;

	//Ball is close
	double angle = target.getAngle();
	if (angle > 180)
		angle = angle - 360;
	if (target.getDistance() < 20) {
		if (abs(angle) <= 13) {
			m_pCom->ToggleTribbler(true);
			return DRIVEMODE_2V2_CATCH_BALL;
		}
		else {
			m_pCom->Drive(0, 0, angle);
			m_pCom->ToggleTribbler(true);
		}

	}
	else {
		if (target.getDistance() > 100){
			speed = 60;
		}
		else{
			speed = target.getDistance(); 
		}
		double angleConst = abs(angle) < 13 ? 0 : std::max(std::min(50.0 / target.getDistance(), 1.0), 0.5);
		std::cout << angleConst << std::endl;
		m_pCom->Drive(speed, angle, angleConst * angle); 
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

	//std::cout << catchDuration << std::endl;
	if (m_pCom->BallInTribbler()) {
		return DRIVEMODE_2V2_AIM_GATE;
	}
	else if (catchDuration > 2000) { //trying to catch ball for 2 seconds
		return DRIVEMODE_2V2_DRIVE_TO_BALL;
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
	return DRIVEMODE_2V2_CATCH_BALL;
}

/*BEGIN DriveHome2v2*/
void DriveHome2v2::onEnter(){
}

NewDriveMode DriveHome2v2::step(double dt){
	ObjectPosition &lastGateLocation = m_pFieldState->GetHomeGate();
	double angle = lastGateLocation.getAngle();
	if (angle > 180)
		angle -= 360;
	if (abs(angle) < 10){
		if (lastGateLocation.getDistance() > 10){
			m_pCom->Drive(90);
		}
		else return DRIVEMODE_2V2_DEFENSIVE;
	}
	else {
		m_pCom->Drive(0, 0, angle);
	}
	return DRIVEMODE_2V2_DRIVE_HOME;
}