#include "MultiModePlay.h"
#include "SingleModePlay.h"
#include "DistanceCalculator.h"
#include "AutoPlayHelpers.h"

const float KICKOFF_ANGLE = 45.;
extern DistanceCalculator gDistanceCalculator;

enum MultiModeDriveStates {
	//2v2 modes
	DRIVEMODE_2V2_OFFENSIVE = 100,
	DRIVEMODE_2V2_DEFENSIVE,
	DRIVEMODE_2V2_WAIT_KICKOFF,
	DRIVEMODE_2V2_CATCH_KICKOFF,
	DRIVEMODE_2V2_AIM_PARTNER,
	//DRIVEMODE_AIM_GATE,
	//DRIVEMODE_KICK,
	//DRIVEMODE_DRIVE_TO_BALL,
	//DRIVEMODE_CATCH_BALL,
	DRIVEMODE_2V2_DRIVE_HOME,
	DRIVEMODE_2V2_OPPONENT_KICKOFF,
	DRIVEMODE_2V2_GOAL_KEEPER

};
class DriveToBallv2 : public DriveToBall
{
public:
	DriveToBallv2(const std::string &name = "DRIVE_TO_BALL_V2") : DriveToBall(name){};

	DriveMode step(double dt)
	{

		auto &target = getClosestBall();

		if (m_pCom->BallInTribbler()) {
			std::cout << "BallInTribbler" << std::endl;
			return DRIVEMODE_AIM_GATE;
		}
		if (driveToTargetWithAngle(target, speed, 25, 5)){return DRIVEMODE_CATCH_BALL;}
		else {
			m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
			return DRIVEMODE_DRIVE_TO_BALL;
		}

	}
};

class CatchBall2v2 : public CatchBall
{
private:
	boost::posix_time::ptime catchStart;
	bool master;
public:
	CatchBall2v2(bool mode) : CatchBall("2V2_CATCH_BALL"), master(mode){};
	virtual DriveMode step(double dt){
			if (m_pCom->BallInTribbler()){
				std::cout << "BallInTribbler" << std::endl;
				if (m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF)return DRIVEMODE_2V2_AIM_PARTNER;
		else return DRIVEMODE_2V2_OFFENSIVE;
			};

	FIND_TARGET_BALL //TODO: use it?
	double heading = target.getHeading();
		if (/*STUCK_IN_STATE(3000) ||*/ target.getDistance() > (initDist + 10)) return DRIVEMODE_DRIVE_TO_BALL;
	speed.velocity, speed.heading, speed.rotation = 0;
	if (fabs(target.getHeading()) <= 2.) {
		if (catchTarget(target, speed)) {
			//if (m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF)return DRIVEMODE_2V2_AIM_PARTNER;
			//else return DRIVEMODE_2V2_OFFENSIVE;
			
		}
		speed.rotation = - sign0(heading) * std::min(40.0, std::max(fabs(heading),5.0));

	}
	else {
		double heading = sign(target.getHeading())*10.;
		//move slightly in order not to get stuck
		speed.velocity = 50;
		speed.rotation = -heading;
	}
	m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	return DRIVEMODE_CATCH_BALL;

/*
	if(m_pCom->BallInTribbler()){
		if (m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF)return DRIVEMODE_2V2_AIM_PARTNER;
		else return DRIVEMODE_2V2_OFFENSIVE;

	}
	FIND_TARGET_BALL //TODO: use it?
	if (STUCK_IN_STATE(3000) || target.getDistance() > initDist  + 10) return DRIVEMODE_DRIVE_TO_BALL;
	speed.velocity, speed.heading, speed.rotation = 0;
	if(fabs(target.getHeading()) <= 2) { 
		if(catchTarget(target, speed)){
			if (m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF)return DRIVEMODE_2V2_AIM_PARTNER;
				else return DRIVEMODE_2V2_OFFENSIVE;

		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_CATCH_BALL;
	}
	double heading = sign(target.getHeading())*10;
	//move slightly in order not to get stuck
	if(heading == 0) m_pCom->Drive(-10,0, 0);
	else m_pCom->Drive(0,0, heading);
	return DRIVEMODE_DRIVE_TO_BALL;
	*/
	}
	
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
		case FieldState::GAME_MODE_START_OUR_THROWIN:
			return m_pFieldState->isPlaying ? DRIVEMODE_DRIVE_TO_BALL : DRIVEMODE_IDLE;		
		}
		return DRIVEMODE_IDLE;
	}
};
class SlaveModeIdle : public Idle {

	virtual DriveMode step(double dt) {
		//while (!m_pFieldState->isPlaying) return DRIVEMODE_IDLE;
		switch (m_pFieldState->gameMode) {
		case FieldState::GAME_MODE_START_OPPONENT_KICK_OFF:
			return DRIVEMODE_2V2_DEFENSIVE;
		case FieldState::GAME_MODE_START_OPPONENT_THROWIN:
			return DRIVEMODE_2V2_DEFENSIVE;
		case FieldState::GAME_MODE_START_OPPONENT_FREE_KICK:
			return DRIVEMODE_2V2_DEFENSIVE;
		case FieldState::GAME_MODE_START_OUR_KICK_OFF:
		case FieldState::GAME_MODE_START_OUR_FREE_KICK:
			return DRIVEMODE_2V2_CATCH_KICKOFF;
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
			//Reverese to GOAL			
			const ObjectPosition &gate = m_pFieldState->GetTargetGate();
			double reverseHeading = gate.getHeading() - 180 * sign(gate.getHeading());
			double targetHeading = gate.getHeading();
			double targetDistance = gate.getDistance();

			double rotation = 0;
			double errorMargin = 5;
			double maxDistance = 80;
			if (fabs(reverseHeading) > errorMargin) rotation = -sign0(reverseHeading) * std::min(30.0, std::max(fabs(reverseHeading), 5.0));
			double heading = 0;
			double speed = 0;
			if (targetDistance > maxDistance) {
				heading = targetHeading;
				if (fabs(heading) > 30) heading = sign0(heading)*(fabs(heading) + 15);
				speed = 60;//std::max(60.0, targetDistance);
			}
			else {
				m_pCom->Drive(0,0,0);
				std::cout<<"to aim gate " << targetDistance << " " << maxDistance<<std::endl;
				return DRIVEMODE_AIM_GATE;
			}
			m_pCom->Drive(speed, heading, rotation);
			return DRIVEMODE_2V2_OFFENSIVE;
		}
		else return DRIVEMODE_CATCH_BALL;
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_2V2_OFFENSIVE;
	}
};

class Defensive : public DriveInstruction
{
public:
	Defensive() : DriveInstruction("2V2_DEFENSIVE"){};
	virtual DriveMode step(double dt){
		auto & target = m_pFieldState->partner;
		if (/*partner not goal keeper become goal keeper*/ false){ return DRIVEMODE_2V2_DRIVE_HOME;}//ToDo goalKeeper message 
		else{
			auto & opponent = m_pFieldState->opponents[0];//get the one with ball?
			//auto & opponent = m_pFieldState->GetTargetGate(); for testing
			auto & homeGate = m_pFieldState->GetHomeGate();
			double gateHeading = homeGate.getHeading() - 180 * sign(homeGate.getHeading());
			double gateAngle = homeGate.getHeading() - 180 * sign(homeGate.getHeading());;
			double opponentAngle = opponent.getAngle();
			double opponentHeading = opponent.getHeading();
			double opponentDistance = opponent.getDistance();
			double rotation = 0;
			double heading = 0;
			double velocity = 0;
			double errorMargin = 5;
			double maxDistance = 30;
			if (fabs(gateHeading) > errorMargin) rotation = -sign(gateHeading) * std::min(40.0, std::max(fabs(gateHeading), 5.0));
			if (opponentDistance > maxDistance) {
				maxDistance = 30;
				double top = 1;
				heading = opponentAngle + sign(opponentHeading) * top*asin(maxDistance / opponentDistance) * 180 / CV_PI;
				velocity = std::max(60.0, opponentDistance);
			}
			else if (fabs(gateHeading - opponentHeading) > errorMargin / 2){
				double top = 1;
				double left = sign(opponentHeading);
				heading = opponentAngle + top*left * 90;
				velocity = 40;
				maxDistance = 60;
			}
			speed.velocity = velocity;
			speed.heading = heading;
			speed.rotation = rotation;
			
		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_2V2_DEFENSIVE;
	}
};

class CatchKickOff : public DriveToBallv2
{
private:
	bool active = false;
public:
	CatchKickOff() : DriveToBallv2("2V2_CATCH_KICKOFF"){};

	virtual DriveMode step(double dt){
		if (m_pFieldState->gameMode == FieldState::GAME_MODE_TAKE_BALL){
			m_pFieldState->gameMode = FieldState::GAME_MODE_IN_PROGRESS;
			return DRIVEMODE_DRIVE_TO_BALL;
		}
		
		return DRIVEMODE_2V2_CATCH_KICKOFF;
	}
};

class AimPartner : public DriveInstruction
{
protected:
	double initialHeading = 0;
public:
	AimPartner() : DriveInstruction("2V2_AIM_PARTNER"){};
	void onEnter(){ 
		initialHeading = m_pFieldState->GetHomeGate().polarMetricCoords.y;
	}
	virtual DriveMode step(double dt){

		//auto & target = m_pFieldState->partner;
		auto target = m_pFieldState->GetHomeGate();
		//std::cout << target.polarMetricCoords.y << std::endl;
		if (aimTarget(target, speed, KICKOFF_ANGLE)){
			//m_pCom->Drive(0, 0, sign(m_pFieldState->self.getHeading())*20);
			//std::this_thread::sleep_for(std::chrono::milliseconds(500));
			m_pCom->Kick(2600);
			m_pFieldState->SendPartnerMessage("PAS #");
			return DRIVEMODE_2V2_DRIVE_HOME;
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
				speed.velocity = 45;
				speed.heading += 90;
			}
			else return DRIVEMODE_KICK;
		}
		else {
			speed.rotation = lastGateLocation.getHeading();
			if(fabs(speed.rotation) > 50) speed.rotation = sign(speed.rotation)*50;				
		}
		m_pCom->Drive(speed.velocity, speed.heading, -speed.rotation);
		return DRIVEMODE_AIM_GATE;
	}
};



class Kick2v2 : public DriveInstruction
{
public:
	Kick2v2() : DriveInstruction("2V2_KICK"){};
	virtual void onEnter();
	virtual DriveMode step(double dt);
};



class DriveHome2v2 : public DriveInstruction
{
public:
	DriveHome2v2() : DriveInstruction("2V2_DRIVE_HOME"){};
		virtual void onEnter(){
			m_pCom->ToggleTribbler(false);
			
		}
	virtual DriveMode step(double dt){
		ObjectPosition &lastGateLocation = m_pFieldState->GetHomeGate();
		if (DriveInstruction::driveToTargetWithAngle(lastGateLocation, speed, 50))return DRIVEMODE_2V2_GOAL_KEEPER;
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
		if (ball.getDistance() > 500) return DRIVEMODE_2V2_DEFENSIVE;
		const BallPosition& newBall = getClosestBall();
		if (abs(ball.getDistance() - newBall.getDistance()) > 10 || abs(ball.getAngle() - newBall.getAngle()) > 5){ return DRIVEMODE_2V2_OFFENSIVE;	}
		else{
			std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
			return DRIVEMODE_2V2_OPPONENT_KICKOFF;
		}
	}
private:
	ObjectPosition ball;
	bool mode;
};

class GoalKeeper : public DriveInstruction
{
private:
	bool wentLeft = false;
public:
	GoalKeeper() : DriveInstruction("2V2_GOAL_KEEPER"){};

	virtual DriveMode step(double dt){
		auto &target = getClosestBall();
		auto homeGate = m_pFieldState->GetHomeGate();
		double gateAngle = homeGate.getHeading() - 180 * sign(homeGate.getHeading());
		aimTarget(target, speed,2);		
		if (gateAngle < 0) {
			speed.heading = 90;
		}
		else {
			speed.heading = -90;
		}
		m_pCom->Drive(50, speed.heading, speed.rotation);
		return DRIVEMODE_2V2_GOAL_KEEPER;
	}
};

std::pair<DriveMode, DriveInstruction*> MasterDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new MasterModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBallv2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall2v2(true)),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DEFENSIVE, new Defensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OFFENSIVE, new Offensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_AIM_PARTNER, new AimPartner()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
//	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICKOFF, new KickOff()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_HOME, new DriveHome2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OPPONENT_KICKOFF, new OpponentKickoff(true)),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_GOAL_KEEPER, new GoalKeeper())
};

std::pair<DriveMode, DriveInstruction*> SlaveDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new SlaveModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBallv2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall2v2(false)),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DEFENSIVE, new Defensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OFFENSIVE, new Offensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBallv2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_HOME, new DriveHome2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_CATCH_KICKOFF, new CatchKickOff()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OPPONENT_KICKOFF, new OpponentKickoff(false)),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_GOAL_KEEPER, new GoalKeeper())
//	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
};

MultiModePlay::MultiModePlay(ICommunicationModule *pComModule, FieldState *pState, bool bMaster) :StateMachine(pComModule, pState,
	bMaster ? TDriveModes(MasterDriveModes, MasterDriveModes + sizeof(MasterDriveModes) / sizeof(MasterDriveModes[0]))
	: TDriveModes(SlaveDriveModes, SlaveDriveModes + sizeof(SlaveDriveModes) / sizeof(SlaveDriveModes[0])))
	, isMaster(bMaster){}


MultiModePlay::~MultiModePlay(){}

/*BEGIN Kick2v2*/
void Kick2v2::onEnter(){
	DriveInstruction::onEnter();
	m_pCom->ToggleTribbler(0);
}

DriveMode Kick2v2::step(double dt){
	m_pCom->ToggleTribbler(0);
	m_pCom->Drive(0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	m_pCom->Kick(2500);
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return DRIVEMODE_2V2_DEFENSIVE;
}
