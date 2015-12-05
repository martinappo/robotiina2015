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
	DRIVEMODE_2V2_DRIVE_HOME,
	DRIVEMODE_2V2_OPPONENT_KICKOFF,
	DRIVEMODE_2V2_GOAL_KEEPER,
	DRIVEMODE_2V2_DRIVE_TO_BALL_NAIVE,
	DRIVEMODE_2V2_CATCH_BALL_NAIVE,
	DRIVEMODE_2V2_DRIVE_TO_BALL_AIM_GATE,
	DRIVEMODE_2V2_AIM_BALL,
	DRIVEMODE_2V2_BACK_UP,
	DRIVEMODE_2V2_DRIVE_TARGET_GATE,
};

class AimBall : public DriveToBall
{
public:

	AimBall(const std::string &name = "AIM_BALL") : DriveToBall(name){};

	DriveMode step(double dt)
	{
		speed = {0,0, 0};
		auto &target = getClosestBall();
		if (aimTarget(target, speed, 6)){
			m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
			return DRIVEMODE_CATCH_BALL;
		}
		//std::cout << "1 " << target.getHeading() << " " << speed.velocity << " " << speed.heading << " " << speed.rotation << std::endl;
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_2V2_AIM_BALL;
	}
};

class BackUp : public DriveToBall
{
public:

	BackUp(const std::string &name = "BACK_UP") : DriveToBall(name){};

	DriveMode step(double dt)
	{		
		if (STUCK_IN_STATE(1000)) {
			return prevDriveMode;
		}

		m_pCom->Drive(-10, lastBallPos.y, 0);
		return DRIVEMODE_2V2_BACK_UP;
	}
};

class DriveToBallNaivev2 : public DriveToBall
{
public:
	int colisionTicker = 0;
	Speed lastSpeed;

	DriveToBallNaivev2(const std::string &name = "DRIVE_TO_BALL_NAIVE") : DriveToBall(name){};

	DriveMode step(double dt)
	{
		speed = {0,0, 0};
		auto &target = getClosestBall();
		if (m_pCom->BallInTribbler(true)) return DRIVEMODE_CATCH_BALL;
		if (aimTarget(target, speed, 10)){
			if (driveToTarget(target, speed, 35)) {
				if (aimTarget(target, speed, 5)) {
					lastBallPos = target.polarMetricCoords;
					return DRIVEMODE_2V2_CATCH_BALL_NAIVE;
				}
			}
		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		lastBallPos = target.polarMetricCoords;
		return DRIVEMODE_2V2_DRIVE_TO_BALL_NAIVE;
	}
};

class CatchBallNaivev2 : public DriveToBall
{
public:
	int colisionTicker = 0;
	Speed lastSpeed;
	double lastHeading ;
	CatchBallNaivev2(const std::string &name = "CATCH_BALL_NAIVE") : DriveToBall(name){};
	void onEnter(){ 
		m_pCom->ToggleTribbler(200);
		//std::cout << "x" << std::endl;
		lastSpeed = {0,0,0};
		lastHeading = getClosestBall().getHeading();
	}

	DriveMode step(double dt)
	{
		if(m_pCom->BallInTribbler(true)) {
			return DRIVEMODE_CATCH_BALL;

		}
		auto &target = getClosestBall();

		//std::cout << "a " << ((boost::posix_time::microsec_clock::local_time() - actionStart)).total_milliseconds()<<std::endl;
		if (STUCK_IN_STATE(1000) /*|| (target.getDistance() > 50*/) {
			//std::cout << "b " << std::endl;
			return DRIVEMODE_2V2_BACK_UP;
		}
		if (target.getHeading() > 15.f && target.getDistance() < 40) {
			//std::cout << "c " << std::endl;
			return DRIVEMODE_2V2_AIM_BALL;
		}
		lastSpeed.velocity = 50; 
		m_pCom->Drive(lastSpeed.velocity, lastSpeed.heading, lastSpeed.rotation);
		lastBallPos = target.polarMetricCoords;
		return DRIVEMODE_2V2_CATCH_BALL_NAIVE;
	}
};

class DriveToBallv2 : public DriveToBall
{
public:
	DriveToBallv2(const std::string &name = "DRIVE_TO_BALL_V2") : DriveToBall(name){};
	void onEnter(){ 
		m_pCom->ToggleTribbler(false);
	}

	DriveMode step(double dt)
	{

		auto &target = getClosestBall();
		if (m_pCom->BallInTribbler(true)) {
			//std::cout << "BallInTribbler" << std::endl;
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
		if (m_pCom->BallInTribbler(true)){
			//std::cout << "BallInTribbler" << std::endl;
			if (m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF)return DRIVEMODE_2V2_AIM_PARTNER;
			else return DRIVEMODE_2V2_OFFENSIVE;
		};

		if(STUCK_IN_STATE(1200)) {
			if (m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF) return DRIVEMODE_2V2_DRIVE_TO_BALL_AIM_GATE;
			else return DRIVEMODE_DRIVE_TO_BALL;
		}

		FIND_TARGET_BALL
		double heading = target.getHeading();
		if (target.getDistance() > (initDist + 10)) return DRIVEMODE_DRIVE_TO_BALL;
		speed.velocity, speed.heading, speed.rotation = 0;
		if (fabs(target.getHeading()) <= 2.) {
			catchTarget(target, speed);//set speed, always returns false
			speed.rotation = - sign0(heading) * std::min(40.0, std::max(fabs(heading),5.0));
		}
		else {
			double heading = sign(target.getHeading())*10.;
			speed.velocity = 50;
			speed.rotation = -heading;
		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_CATCH_BALL;
	}
	
};

class MasterModeIdle : public Idle {

	virtual DriveMode step(double dt) {
				if (!m_pFieldState->isPlaying) return DRIVEMODE_IDLE;

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
		return DRIVEMODE_2V2_DRIVE_TO_BALL_AIM_GATE;
//        return DRIVEMODE_2V2_DRIVE_TO_BALL_NAIVE;

		}
		return DRIVEMODE_IDLE;
	}
};
class SlaveModeIdle : public Idle {

	virtual DriveMode step(double dt) {
		if (!m_pFieldState->isPlaying) return DRIVEMODE_IDLE;
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
		if (m_pCom->BallInTribbler(true)){
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
		if (!m_pFieldState->partnerGoalKeeper){ 
			m_pFieldState->SendPartnerMessage("GLK #");
			return DRIVEMODE_2V2_DRIVE_HOME; }
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
		if (m_pFieldState->gameMode == FieldState::GAME_MODE_TAKE_BALL) {
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
			m_pCom->Drive(0, 0, sign(m_pFieldState->self.getHeading())*20);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			if(m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF) {
				m_pFieldState->SendPartnerMessage("PAS #");
				m_pCom->Kick(1500);
			} else 
				m_pCom->Kick(5000);
				
			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
			if (m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF) return DRIVEMODE_2V2_DRIVE_TARGET_GATE;//decoy
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
		if (!m_pCom->BallInTribbler(true)) return DRIVEMODE_2V2_OFFENSIVE;
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


class DriveTargetGate2v2 : public DriveInstruction
{
public:
	DriveTargetGate2v2() : DriveInstruction("2V2_TO_TARGET_GATE"){};
	virtual DriveMode step(double dt){
		ObjectPosition &lastGateLocation = m_pFieldState->GetTargetGate();
		if (DriveInstruction::driveToTargetWithAngle(lastGateLocation, speed, 95))return DRIVEMODE_2V2_DRIVE_TARGET_GATE;
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_2V2_DRIVE_TARGET_GATE;
	}
};


class DriveHome2v2 : public DriveInstruction
{
public:
	DriveHome2v2() : DriveInstruction("2V2_DRIVE_HOME"){};
	virtual DriveMode step(double dt){
		ObjectPosition &lastGateLocation = m_pFieldState->GetHomeGate();
		if (DriveInstruction::driveToTargetWithAngle(lastGateLocation, speed, 65))return DRIVEMODE_2V2_GOAL_KEEPER;
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
		if(m_pCom->BallInTribbler(true)) {
			m_pFieldState->SendPartnerMessage("GL2O #");
			return DRIVEMODE_2V2_OFFENSIVE;
		}
		auto &target = getClosestBall();
		auto homeGate = m_pFieldState->GetHomeGate();
		auto targetGate = m_pFieldState->GetTargetGate();
		double homeGateDist = homeGate.getDistance();
		double gateAngle = homeGate.getHeading() - 180 * sign(homeGate.getHeading());
		if (target.getDistance() == 0.0) {
			speed = { 0, 0, 0 };
			m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
			return DRIVEMODE_2V2_GOAL_KEEPER;
		}
		// if we are to close to gate
		if(m_pFieldState->collisionWithUnknown) {
			m_pCom->Drive(20, targetGate.getHeading()+45, 0);
			return DRIVEMODE_2V2_GOAL_KEEPER;
			
		}
		if (target.getDistance() < 35 && !m_pFieldState->obstacleNearBall) {
			m_pCom->Drive(0,0,0);
			return DRIVEMODE_2V2_GOAL_KEEPER;
		}
		aimTarget(target, speed,2);	
		if (m_pFieldState->collisionWithUnknown || homeGateDist < 30 || homeGate.minCornerPolarCoords.x < 30) {
			driveToTargetWithAngle(target, speed, 40, 5);
			speed.velocity = 30;
		} else {	
			if(m_pFieldState->collisionWithUnknown) {
				speed.heading = targetGate.getHeading();
			}else  if (gateAngle < 0) {
				speed.heading = 90;
			}
			else {
				speed.heading = -90;
			}
			speed.velocity = 50;
		}

		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_2V2_GOAL_KEEPER;
	}
};

class DriveToBallAimGate2v2 : public DriveInstruction
{
public:
	DriveToBallAimGate2v2(const std::string &name = "2V2_DRIVE_TO_BALL_AIM_GATE") : DriveInstruction(name) {};

	DriveMode step(double dt) {
		if (m_pCom->BallInTribbler()){
			if (m_pFieldState->gameMode == FieldState::GAME_MODE_START_OUR_KICK_OFF)return DRIVEMODE_2V2_AIM_PARTNER;
			else return DRIVEMODE_2V2_OFFENSIVE;
		}
		const ObjectPosition &ball = getClosestBall();
		const ObjectPosition &gate = m_pFieldState->GetHomeGate();
		double gateHeading = gate.getHeading();
		double ballHeading = ball.getHeading();
		double ballDistance = ball.getDistance();
		double rotation = 0;
		double errorMargin = 5;
		double maxDistance = 40;
		if (fabs(gateHeading) > errorMargin) rotation = -sign0(gateHeading) * std::min(40.0, std::max(fabs(gateHeading), 5.0));
		double heading = 0;
		double speed = 0;
		if (ballDistance > maxDistance) {
			heading = ballHeading;// +sign(gateHeading) / ballDistance;
			if (fabs(heading) > 30) heading = sign0(heading)*(fabs(heading) + 15);
			speed = std::max(60.0, ballDistance);
		}
		else {
			if (fabs(ballHeading) <= errorMargin && fabs(gateHeading) <= errorMargin) {
				m_pCom->Drive(0, 0, 0);
				return DRIVEMODE_CATCH_BALL;
			}
			if (fabs(ballHeading) > errorMargin) {
				heading = ballHeading + sign0(ballHeading) * 45;
				speed = 35;
			}
			rotation = 0;
			if (fabs(gateHeading) > errorMargin) rotation = -sign0(gateHeading) * std::min(40.0, std::max(fabs(gateHeading), 5.0));
		}
		m_pCom->Drive(speed, heading, rotation);
		return DRIVEMODE_2V2_DRIVE_TO_BALL_AIM_GATE;
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
//	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_KICKOFF, new KickOff()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_HOME, new DriveHome2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OPPONENT_KICKOFF, new OpponentKickoff(true)),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_GOAL_KEEPER, new GoalKeeper()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_TO_BALL_NAIVE, new DriveToBallNaivev2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_TO_BALL_AIM_GATE, new DriveToBallAimGate2v2()),
    std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_CATCH_BALL_NAIVE, new CatchBallNaivev2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_AIM_BALL, new AimBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_BACK_UP, new BackUp()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_TARGET_GATE, new DriveTargetGate2v2()),

};

std::pair<DriveMode, DriveInstruction*> SlaveDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new SlaveModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBallAimGate2v2()), //new DriveToBallv2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall2v2(false)),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DEFENSIVE, new Defensive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OFFENSIVE, new Offensive()),
	//std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBallv2()), // duplicate
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_HOME, new DriveHome2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate2v2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_CATCH_KICKOFF, new CatchKickOff()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_OPPONENT_KICKOFF, new OpponentKickoff(false)),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_GOAL_KEEPER, new GoalKeeper()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_TO_BALL_NAIVE, new DriveToBallNaivev2()),
    std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_CATCH_BALL_NAIVE, new CatchBallNaivev2()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_AIM_BALL, new AimBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_BACK_UP, new BackUp()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_2V2_DRIVE_TARGET_GATE, new DriveTargetGate2v2()),
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
	m_pCom->Kick(5000);
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return DRIVEMODE_2V2_DEFENSIVE;
}
