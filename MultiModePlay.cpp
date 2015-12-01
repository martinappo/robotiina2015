#include "MultiModePlay.h"
#include "SingleModePlay.h"
#include "DistanceCalculator.h"

extern DistanceCalculator gDistanceCalculator;

enum MultiModeDriveStates {
	//2v2 modes
	DRIVEMODE_2V2_OFFENSIVE = 100,
	DRIVEMODE_2V2_DEFENSIVE,
	DRIVEMODE_2V2_KICKOFF,
	DRIVEMODE_2V2_AIM_PARTNER,
	DRIVEMODE_2V2_AIM_GATE,
	DRIVEMODE_2V2_KICK,
	DRIVEMODE_2V2_DRIVE_TO_BALL,
	DRIVEMODE_2V2_CATCH_BALL,
	DRIVEMODE_2V2_DRIVE_HOME,
	DRIVEMODE_2V2_OPPONENT_KICKOFF

};

class MasterModeIdle : public Idle {

	virtual DriveMode step(double dt) {
		switch (m_pFieldState->gameMode) {
		case FieldState::GAME_MODE_START_OPPONENT_KICK_OFF:
			return DRIVEMODE_2V2_OPPONENT_KICKOFF;
		case FieldState::GAME_MODE_START_OPPONENT_THROWIN:
			return DRIVEMODE_2V2_OPPONENT_KICKOFF;
		case FieldState::GAME_MODE_START_OPPONENT_FREE_KICK:
			return DRIVEMODE_2V2_OPPONENT_KICKOFF;
		case FieldState::GAME_MODE_START_OUR_KICK_OFF:
		case FieldState::GAME_MODE_START_OUR_FREE_KICK:
			m_pFieldState->gameMode = FieldState::GAME_MODE_IN_PROGRESS;
			return DRIVEMODE_2V2_KICKOFF;
		case FieldState::GAME_MODE_START_OUR_THROWIN:
			return DRIVEMODE_DRIVE_TO_BALL;
		}
		return DRIVEMODE_IDLE;
	}
};
class SlaveModeIdle : public Idle {

	virtual DriveMode step(double dt) {
		switch (m_pFieldState->gameMode) {
		case FieldState::GAME_MODE_START_OPPONENT_KICK_OFF:
			return DRIVEMODE_2V2_DEFENSIVE;
		case FieldState::GAME_MODE_START_OPPONENT_THROWIN:
			return DRIVEMODE_2V2_DEFENSIVE;
		case FieldState::GAME_MODE_START_OPPONENT_FREE_KICK:
			return DRIVEMODE_2V2_DEFENSIVE;
		case FieldState::GAME_MODE_START_OUR_KICK_OFF:
			return DRIVEMODE_2V2_KICKOFF;
		case FieldState::GAME_MODE_START_OUR_FREE_KICK:
			m_pFieldState->gameMode = FieldState::GAME_MODE_IN_PROGRESS;
			return DRIVEMODE_2V2_KICKOFF;
		case FieldState::GAME_MODE_START_OUR_THROWIN:
			return DRIVEMODE_2V2_DEFENSIVE;
		}
		return DRIVEMODE_IDLE;
	}
};
class Offensive : public DriveInstruction
{
public:
	Offensive() : DriveInstruction("2V2_OFFENSIVE"){};
	virtual DriveMode step(double dt){
		if (m_pCom->BallInTribbler()){
			//ATTACK! -> GOAL!!
			return DRIVEMODE_2V2_AIM_GATE;
		}
		else
			return DRIVEMODE_2V2_AIM_GATE;
		return DRIVEMODE_2V2_OFFENSIVE;
	}
};

class Defensive : public DriveInstruction
{
public:
	Defensive() : DriveInstruction("2V2_DEFENSIVE"){};
	virtual DriveMode step(double dt){

		auto & target = m_pFieldState->partner;
		if (m_pFieldState->partnerHomeGate.getDistance() > 100){//is ally in defense area?
			if (m_pFieldState->GetHomeGate().getDistance() > 80)
				return DRIVEMODE_2V2_DRIVE_HOME;
			else{
				BallPosition ball = DriveInstruction::getClosestBall();
				if (ball.polarMetricCoords.x != INT_MAX)
					DriveInstruction::aimTarget(ball, speed);
				//block gate & look for ball
			}
		}
		else{
			/*active defense
			positon self between opponent and gate?
			*/
		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_2V2_DEFENSIVE;
	}
};

class KickOff : public DriveToBall
{
private:
	bool active = false;
public:
	KickOff() : DriveToBall("2V2_KICKOFF"){};
	virtual DriveMode step(double dt){
		DriveMode next = DriveToBall::step(dt);
		switch (next)
		{
		case DRIVEMODE_AIM_GATE:
			return DRIVEMODE_2V2_AIM_PARTNER;
		default:
			return DRIVEMODE_2V2_KICKOFF;
		} 
	}
};
class AimPartner : public DriveInstruction
{
public:
	AimPartner() : DriveInstruction("2V2_AIM_PARTNER"){};
	virtual DriveMode step(double dt){

		auto & target = m_pFieldState->partner;
		std::cout << target.polarMetricCoords.y << std::endl;
		if (aimTarget(target, speed, 2)){
			m_pCom->Kick(400);
			std::cout << "kicked" << std::endl;
			m_pFieldState->SendMessage("PAS #");
			std::cout << DRIVEMODE_2V2_DEFENSIVE << std::endl;
			return DRIVEMODE_2V2_DEFENSIVE;
		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_2V2_AIM_PARTNER;
	}
};


class AimGate2v2 : public DriveInstruction
{
public:
	AimGate2v2() : DriveInstruction("2V2_AIM_GATE"){};
	virtual DriveMode step(double dt){

		ObjectPosition &lastGateLocation = m_pFieldState->GetTargetGate();
		bool sightObstructed = m_pFieldState->gateObstructed;
		if (!m_pCom->BallInTribbler()) return DRIVEMODE_2V2_OFFENSIVE;
		if (aimTarget(lastGateLocation, speed, 2)){
			if (sightObstructed) { //then move sideways away from gate
				//std::cout << sightObstructed << std::endl;
				speed.velocity = 45;
				speed.heading += 90;
//				std::chrono::milliseconds dura(400); // do we need to sleep?
//				std::this_thread::sleep_for(dura);
			}
			else {
				return DRIVEMODE_2V2_KICK;
			}
		}
		else {
			speed.rotation = lastGateLocation.getHeading();
		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);

		return DRIVEMODE_2V2_AIM_GATE;
	}
};



class Kick2v2 : public DriveInstruction
{
public:
	Kick2v2() : DriveInstruction("2V2_KICK"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
};
/*
class DriveToBall2v2 : public DriveInstruction
{
private:
	TargetPosition start;
	BallPosition target;
	double speed;
	double rotate;
	double rotateGate;
	int desiredDistance = 210;
public:
	DriveToBall2v2() : DriveInstruction("2V2_DRIVE_TO_BALL"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
};

class CatchBall2v2 : public DriveInstruction
{
private:
	boost::posix_time::ptime catchStart;
public:
	CatchBall2v2() : DriveInstruction("2V2_CATCH_BALL"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
};
*/
class DriveHome2v2 : public DriveInstruction
{
public:
	DriveHome2v2() : DriveInstruction("2V2_DRIVE_HOME"){};
	virtual DriveMode step(double dt){

		ObjectPosition &lastGateLocation = m_pFieldState->GetHomeGate();
		if (DriveInstruction::aimTarget(lastGateLocation, speed)){
			if (DriveInstruction::driveToTarget(lastGateLocation, speed)){
				if (DriveInstruction::aimTarget(lastGateLocation, speed)){
					return DRIVEMODE_2V2_DEFENSIVE;
				}
			}
		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);

		return DRIVEMODE_2V2_DRIVE_HOME;
	}
};

class OpponentKickoff : public DriveInstruction
{
public:
	OpponentKickoff(bool mode) : DriveInstruction("2V2_OPPONENT_KICKOFF"), mode(mode){};
	void onEnter(){
		ball = getClosestBall();
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	}

	DriveMode step(double dt){

		if (ball.getDistance() > 500)
			return DRIVEMODE_2V2_DEFENSIVE;
		const BallPosition& newBall = getClosestBall();
		if (abs(ball.getDistance() - newBall.getDistance()) > 10 || abs(ball.getAngle() - newBall.getAngle()) > 5){ //ball has moved
			return DRIVEMODE_2V2_OFFENSIVE;
		}
		else{
			std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
			return DRIVEMODE_2V2_OPPONENT_KICKOFF;
		}
	}
private:
	ObjectPosition ball;
	bool mode;
};

std::pair<DriveMode, DriveInstruction*> MasterDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new MasterModeIdle()),
//	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DEFENSIVE, new Defensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OFFENSIVE, new Offensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_AIM_PARTNER, new AimPartner()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_AIM_GATE, new AimGate2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
//	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_KICKOFF, new KickOff()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_HOME, new DriveHome2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OPPONENT_KICKOFF, new OpponentKickoff(true)),
};

std::pair<DriveMode, DriveInstruction*> SlaveDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new SlaveModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DEFENSIVE, new Defensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OFFENSIVE, new Offensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_HOME, new DriveHome2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_AIM_GATE, new AimGate2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OPPONENT_KICKOFF, new OpponentKickoff(false)),
//	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
};

MultiModePlay::MultiModePlay(ICommunicationModule *pComModule, FieldState *pState, bool bMaster) :StateMachine(pComModule, pState,
	bMaster ? TDriveModes(MasterDriveModes, MasterDriveModes + sizeof(MasterDriveModes) / sizeof(MasterDriveModes[0]))
	: TDriveModes(SlaveDriveModes, SlaveDriveModes + sizeof(SlaveDriveModes) / sizeof(SlaveDriveModes[0])))
	, isMaster(bMaster)
{
}


MultiModePlay::~MultiModePlay()
{
}




/*BEGIN Kick2v2*/
void Kick2v2::onEnter(){
	DriveInstruction::onEnter();
	m_pCom->ToggleTribbler(0);
}

DriveMode Kick2v2::step(double dt){
	m_pCom->ToggleTribbler(0);
	m_pCom->Drive(0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	m_pCom->Kick();
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return DRIVEMODE_2V2_DEFENSIVE;
}

/*BEGIN DriveToBall2v2*/
/*
void DriveToBall2v2::onEnter(){
	DriveInstruction::onEnter();
	m_pCom->Drive(0, 0, 0);
	std::chrono::milliseconds dura(200);
	std::this_thread::sleep_for(dura);
	m_pCom->ToggleTribbler(0);

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
*/
/*BEGIN CatchBall2v2*/
/*
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
*/
/*BEGIN DriveHome2v2*/

