#include "SingleModePlay.h"
#include "AutoPlayHelpers.h"

DriveMode DriveToBall::step(double dt)
{

	auto target = getClosestBall();

	if (target.getDistance() > 10000) return DRIVEMODE_IDLE;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;

	if (aimTarget(target)){
		if (driveToTarget(target)){
			if (aimTarget(target)){
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

	START_TRIBBLER
	STOP_DRIVING
}
void CatchBall::onExit()
{
	STOP_TRIBBLER
}
DriveMode CatchBall::step(double dt)
{


	FIND_TARGET_BALL

	//std::cout << catchDuration << std::endl;
	if (BALL_IN_TRIBBLER)  return DRIVEMODE_AIM_GATE;
	else if (STUCK_IN_STATE(2000)) return DRIVEMODE_DRIVE_TO_BALL;
	else ROTATE_AND_DRIVE_TOWARD_TO_TARGET_SLOWLY
	return DRIVEMODE_CATCH_BALL;
}
/*END CatchBall*/


/*BEGIN AimGate*/
DriveMode AimGate::step(double dt)
{

	FIND_TARGET_GATE


	if (BALL_IN_TRIBBLER) return DRIVEMODE_DRIVE_TO_BALL;

	if (STUCK_IN_STATE(9000)) return DRIVEMODE_KICK;

	//Turn robot to gate
	if (aimTarget(target, 2)){
		if (SIGHT_OBSTRUCTED) { //then move sideways away from gate
			DRIVE_SIDEWAYS
			//m_pCom->Drive(45, 90, 0);

		}
		else {
			return DRIVEMODE_KICK;
		}
	}
	else {
		ROTATE_AND_DRIVE_TOWARD_TO_TARGET_GATE
	}
	return DRIVEMODE_AIM_GATE;

}


/*BEGIN Kick*/
DriveMode Kick::step(double dt)
{
	m_pCom->ToggleTribbler(false);
	STOP_DRIVING
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	m_pCom->Kick();
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return DRIVEMODE_DRIVE_TO_BALL;

}
void Kick::onEnter()
{
	DriveInstruction::onEnter();
	STOP_TRIBBLER
}

std::pair<DriveMode, DriveInstruction*> SingleDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new SingleModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
};

SingleModePlay::SingleModePlay(ICommunicationModule *pComModule, FieldState *pState)
		:StateMachine(pComModule, pState, 
			TDriveModes(SingleDriveModes, SingleDriveModes + sizeof(SingleDriveModes) / sizeof(SingleDriveModes[0])))
{
};

/*2v2 part*/
/*BEGIN Offensive*/
DriveMode Offensive::step(double dt){
	if (m_pCom->BallInTribbler()){
		//ATTACK! -> GOAL!!
		return DRIVEMODE_2V2_AIM_GATE;
	}
	else
		return DRIVEMODE_2V2_CATCH_BALL;
	return DRIVEMODE_2V2_KICK;
}

/*BEGIN Defensive*/
DriveMode Defensive::step(double dt){
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
DriveMode KickOff::step(double dt){
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
DriveMode AimGate2v2::step(double dt){


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

DriveMode Kick2v2::step(double dt){
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

DriveMode DriveToBall2v2::step(double dt){
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

DriveMode CatchBall2v2::step(double dt){
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

DriveMode DriveHome2v2::step(double dt){
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