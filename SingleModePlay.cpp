#include "SingleModePlay.h"
#include "AutoPlayHelpers.h"
#define ANDRESE_CATCH_BALL

enum {
	DRIVEMODE_DRIVE_TO_BALL_NAIVE = 5000,
	DRIVEMODE_DIRVE_TO_BALL_AVOID_TURN,
	DRIVEMODE_DRIVE_TO_BALL_ANGLED,
	DRIVEMODE_DRIVE_TO_BALL_AIM_GATE,
	DRIVEMODE_ROTATE_AROUND_BALL
};

/*BEGIN DriveToBall*/
void DriveToBall::onEnter()
{
	toggledDribbler = false;
	DriveInstruction::onEnter();	
	initialBall = getClosestBall(false, true);
	initialGate = m_pFieldState->GetTargetGate();
	if (ACTIVE_DRIVE_TO_BALL_MODE == DRIVEMODE_IDLE)
		ACTIVE_DRIVE_TO_BALL_MODE = DRIVEMODE_DRIVE_TO_BALL_AIM_GATE;
}

DriveMode DriveToBall::step(double dt){
	if(initialBall.getDistance() == 0) return DRIVEMODE_DRIVE_HOME;
	//return DRIVEMODE_DRIVE_TO_BALL_ANGLED;
	//return DRIVEMODE_DRIVE_TO_BALL_NAIVE;
	//return DRIVEMODE_ROTATE_AROUND_BALL;
	return ACTIVE_DRIVE_TO_BALL_MODE;
}
class DriveToBallNaive : public DriveToBall
{ 
public:
	int colisionTicker = 0;
	Speed lastSpeed;
	DriveToBallNaive(const std::string &name = "DRIVE_TO_BALL_NAIVE") : DriveToBall(name){};
	boost::posix_time::ptime collisionTime = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime collisionTime2 = boost::posix_time::microsec_clock::local_time();

	DriveMode step(double dt)
	{	
		boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type collisionDt = (time - collisionTime).total_milliseconds();
		boost::posix_time::time_duration::tick_type collisionDt2 = (time - collisionTime2).total_milliseconds();

		if (collisionDt < 1000) speed = lastSpeed;
		else {
			auto &target = getClosestBall();
			if (target.getDistance() > 10000) return DRIVEMODE_IDLE;
			if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;
			if (aimTarget(target, speed, 10))
				if (driveToTarget(target, speed, 35))
					if (aimTarget(target, speed, 1))
						return DRIVEMODE_CATCH_BALL;
			if (m_pFieldState->collisionWithUnknown && fabs(speed.velocity) > 1.) {
				if (m_pFieldState->collisionRange.x < m_pFieldState->collisionRange.y) {
					if (speed.heading > m_pFieldState->collisionRange.x) {
						speed.heading -= 90;
						speed.velocity = 100;
					}
					else if (speed.heading < m_pFieldState->collisionRange.y){
						speed.heading += 90;
						speed.velocity = 100;
					}
				}
				speed.rotation = 0;
				collisionTime = boost::posix_time::microsec_clock::local_time();
				lastSpeed = speed;
			}
		}
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);		
		return DRIVEMODE_DRIVE_TO_BALL_NAIVE;
	}
};

class DriveToBallAngled : public DriveToBall
{
public:
	DriveToBallAngled(const std::string &name = "DRIVE_TO_BALL_ANGLED") : DriveToBall(name){};

	DriveMode step(double dt)
	{
		auto &target = getClosestBall();

		if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;
		if (driveToTargetWithAngle(target, speed, 25, 5)){return DRIVEMODE_CATCH_BALL;}
		else {
			m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
			return DRIVEMODE_DRIVE_TO_BALL_ANGLED;
		}
	}
};

class DriveToBallAvoidTurn : public DriveToBall
{
public:
	DriveToBallAvoidTurn(const std::string &name = "DRIVE_TO_BALL_AVOID_TURN") : DriveToBall(name){};

	DriveMode step(double dt)
	{
		
		const ObjectPosition &target = getClosestBall(true);
		//if we are between closest ball and target gate and facing target gate then drive to home
		//to avoid rotating on any next ball
		if (fabs(m_pFieldState->GetTargetGate().getHeading()) < 20
			&& fabs(target.getHeading()) > 120) {
			return DRIVEMODE_DRIVE_HOME;
		}
		if (target.getDistance() > 10000) return DRIVEMODE_IDLE;
		if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;
		//std::cout << std::endl << "aimtarget0, ";
		if (aimTarget(target, speed, 10))
			if (driveToTarget(target, speed))
				if (aimTarget(target, speed, 2)) return DRIVEMODE_CATCH_BALL;
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_DIRVE_TO_BALL_AVOID_TURN;

	}
};
class DriveToBallAimGate : public DriveInstruction
{
public:
	DriveToBallAimGate(const std::string &name = "DRIVE_TO_BALL_AIM_GATE") : DriveInstruction(name){};

	DriveMode step(double dt){
		if (m_pCom->BallInTribbler())return DRIVEMODE_AIM_GATE;
		if (m_pFieldState->obstacleNearBall) {
			ACTIVE_DRIVE_TO_BALL_MODE = DRIVEMODE_DRIVE_TO_BALL_ANGLED;
			return DRIVEMODE_DRIVE_TO_BALL_ANGLED;
		}
		/*
		if (m_pFieldState->collisionWithBorder) {
			ACTIVE_DRIVE_TO_BALL_MODE = DRIVEMODE_DRIVE_TO_BALL_NAIVE;
			return DRIVEMODE_DRIVE_TO_BALL_NAIVE;
		}
		*/
		const ObjectPosition &ball = getClosestBall();
		const ObjectPosition &gate = m_pFieldState->GetTargetGate();
		double gateHeading = gate.getHeading();
		double ballHeading = ball.getHeading();
		double ballDistance = ball.getDistance();
		double rotation = 0;
		double errorMargin = 5;
		double maxDistance = 45;
		if (fabs(gateHeading) > errorMargin) rotation = -sign0(gateHeading) * std::min(40.0, std::max(fabs(gateHeading), 5.0));
		double heading = 0;
		double speed = 0;
		if (ballDistance > maxDistance) {
			heading = ballHeading;// +sign(gateHeading) / ballDistance;
			if (fabs(heading) > 30) heading = sign0(heading)*(fabs(heading) + 15);
			speed = std::max(80.0, ballDistance);
		}
		else {
			if (fabs(ballHeading) <= errorMargin && fabs(gateHeading) <= errorMargin){
				m_pCom->Drive(0, 0, 0);
				return DRIVEMODE_CATCH_BALL;
			}
			if (fabs(ballHeading) > errorMargin){
				heading = ballHeading + sign0(ballHeading) * 45;
				speed = 35;
			}
			rotation = 0;
			if (fabs(gateHeading) > errorMargin) rotation = -sign0(gateHeading) * std::min(40.0, std::max(fabs(gateHeading), 5.0));
			// drive around the ball
			//heading = ballHeading + sign(ballHeading) * 90;
			//std::max(fabs(ballHeading), 35.0);
		}
		m_pCom->Drive(speed, heading, rotation);
		return DRIVEMODE_DRIVE_TO_BALL_AIM_GATE;
	}
};
class DriveToHome : public DriveInstruction
{
public:
	DriveToHome(const std::string &name = "DRIVE_HOME") : DriveInstruction(name){};
	virtual DriveMode step(double dt){
		if(m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;
		auto target = m_pFieldState->GetHomeGate();
		if (target.getDistance() < 80){
			m_pCom->Drive(0,0,0);	
			return DRIVEMODE_DRIVE_TO_BALL;
		} 
		else m_pCom->Drive(80, target.getHeading());
	return DRIVEMODE_DRIVE_HOME;
	}
};

class DriveHomeAtStart : public DriveInstruction
{
public:
	DriveHomeAtStart(const std::string &name = "DRIVE_HOME_AT_START") : DriveInstruction(name){};
	virtual DriveMode step(double dt){
		auto target = m_pFieldState->GetHomeGate();
		const ObjectPosition &ball = getClosestBall();
		if (target.getDistance() < 90 || target.getDistance()/2 > ball.getDistance()){
			m_pCom->Drive(0, 0, 0);
			return DRIVEMODE_DRIVE_TO_BALL;
		}
		//else m_pCom->Drive(90, 0, -sign0(target.getHeading())*20);
		else{
			const ObjectPosition &homeGate = m_pFieldState->GetHomeGate();
			const ObjectPosition &gate = m_pFieldState->GetTargetGate();
			double gateHeading = gate.getHeading();
			double ballHeading = sign(homeGate.getHeading())*(fabs(homeGate.getHeading())-35) ;
			double rotation = 0;
			double errorMargin = 5;
			if (fabs(gateHeading) > errorMargin) rotation = -sign0(gateHeading) * std::min(40.0, std::max(fabs(gateHeading), 5.0));
			double heading = 0;
			double speed = 0;
			heading = ballHeading;// +sign(gateHeading) / ballDistance;
			if (fabs(heading) > 30) heading = sign0(heading)*(fabs(heading) + 15);
			speed = 80;//std::min(30.0,std::max(60.0, target.getDistance()));//limited speed			
			m_pCom->Drive(speed, heading, 0);
		}
	return DRIVEMODE_DRIVE_HOME_AT_START;
	}
};



/*BEGIN CatchBall*/
void CatchBall::onEnter()
{
	DriveInstruction::onEnter();
	m_pCom->ToggleTribbler(250);
	FIND_TARGET_BALL
	initDist = target.getDistance();
	STOP_DRIVING
}
#ifdef ANDRESE_CATCH_BALL
DriveMode CatchBall::step(double dt)
{
	if (m_pCom->BallInTribbler())return DRIVEMODE_AIM_GATE;

	FIND_TARGET_BALL //TODO: use it?
	double heading = target.getHeading();
		if (/*STUCK_IN_STATE(3000) ||*/ target.getDistance() > (initDist + 10)) return DRIVEMODE_DRIVE_TO_BALL;
	speed.velocity, speed.heading, speed.rotation = 0;
	if (fabs(target.getHeading()) <= 2.) {
		if (catchTarget(target, speed)) return DRIVEMODE_AIM_GATE;
		speed.rotation = - sign0(heading) * std::min(40.0, std::max(fabs(heading),5.0));

	}
	else {
		double heading = sign(target.getHeading())*10.;
		//move slightly in order not to get stuck
		speed.velocity = 50;
		speed.rotation = -heading;
	}
	m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
	return DRIVEMODE_CATCH_BALL;
}
#else
DriveMode CatchBall::step(double dt)
{
	FIND_TARGET_BALL //TODO: use it?
	if (STUCK_IN_STATE(3000) || target.getDistance() > initDist  + 10) return DRIVEMODE_DRIVE_TO_BALL;
	speed.velocity, speed.heading, speed.rotation = 0;
	if(fabs(target.getHeading()) <= 2) { 
		if(catchTarget(target, speed)) return DRIVEMODE_AIM_GATE;
		m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
		return DRIVEMODE_CATCH_BALL;
	}
	double heading = sign(target.getHeading())*10;
	//move slightly in order not to get stuck
	if(heading == 0) m_pCom->Drive(-10,0, 0);
	else m_pCom->Drive(0,0, heading);
	return DRIVEMODE_DRIVE_TO_BALL;
}
#endif
void CatchBall::onExit(){}//DO_NOT_STOP_TRIBBLER
/*END CatchBall*/


/*BEGIN AimGate*/

DriveMode AimGate::step(double dt)
{
	FIND_TARGET_GATE
	if (!BALL_IN_TRIBBLER) return DRIVEMODE_DRIVE_TO_BALL;	
	double errorMargin;
	if (target.getDistance() > 200) errorMargin = 1;
	else errorMargin = 2;	
	if (aimTarget(target, speed, errorMargin)) {
		if (m_pFieldState->gateObstructed) {
			double gateAngle = target.getHeading() - 180 * sign(target.getHeading());
			if (gateAngle < 0) {
				speed.heading = -90;
			}
			else {
				speed.heading = 90;
			}
			speed.velocity = 50;
		}
		else {
			m_pCom->Drive(0, 0, 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			if (aimTarget(target, speed, errorMargin))return DRIVEMODE_KICK;
		}
	}
	m_pCom->Drive(speed.velocity, speed.heading, speed.rotation);
	return DRIVEMODE_AIM_GATE;
}


/*BEGIN Kick*/ 
void Kick::onEnter()
{
	DriveInstruction::onEnter();
	STOP_TRIBBLER 
	STOP_DRIVING
	ACTIVE_DRIVE_TO_BALL_MODE = DRIVEMODE_DRIVE_TO_BALL_AIM_GATE;
}
DriveMode Kick::step(double dt)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	m_pCom->Kick(5000);
	std::this_thread::sleep_for(std::chrono::milliseconds(50)); //less than half second wait.
	return DRIVEMODE_DRIVE_TO_BALL;
}

class RotateAroundBall : public DriveInstruction
{
protected:
	double initialBallHeading = 0;
public:
	RotateAroundBall(const std::string &name = "ROTATE_AROUND_BALL") : DriveInstruction(name){};
	void onEnter(){ initialBallHeading = getClosestBall().getHeading();}

	virtual DriveMode step(double dt){
		if (m_pCom->BallInTribbler())return DRIVEMODE_AIM_GATE;
		const ObjectPosition &ball = getClosestBall();
		const ObjectPosition &gate = m_pFieldState->GetTargetGate();
		double gateHeading = gate.getHeading();
		double gateAngle = gate.getAngle();
		double ballAngle = ball.getAngle();
		double ballHeading = ball.getHeading();
		double ballDistance = ball.getDistance();
		double rotation = 0;
		double heading = 0;
		double speed = 0;
		double errorMargin = 5;
		double maxDistance = 30;
		if (fabs(gateHeading) <= errorMargin && fabs(ballHeading) <= errorMargin) return DRIVEMODE_CATCH_BALL;
		if (fabs(gateHeading) > errorMargin) rotation = -sign(gateHeading) * std::min(40.0, std::max(fabs(gateHeading), 5.0));
		if (ballDistance > maxDistance) {
			maxDistance = 30;
			double top = 1;// (fabs(initialBallHeading) > 90) ? -1 : 1;
			heading = ballAngle + sign(ballHeading) * top*asin(maxDistance / ballDistance) * 180 / CV_PI;
			speed = std::max(60.0, ballDistance);
		}
		else if (fabs(gateHeading - ballHeading) > errorMargin/2){
			//return DRIVEMODE_CATCH_BALL;
			// drive around the ball
			double top = 1;// (fabs(initialBallHeading) > 90) ? -1 : 1;
			double left = sign(initialBallHeading);
			heading = ballAngle + top*left*90;
			speed = 40;
			maxDistance = 60;
		}
		if (((speed) < 0.01) && (fabs(heading) < 0.01) && (fabs(rotation) < 0.01)) return DRIVEMODE_CATCH_BALL;// nowhere to go, error margins are out-of-sync
		m_pCom->Drive(speed, heading, rotation);
		return DRIVEMODE_ROTATE_AROUND_BALL;
	}
};



std::pair<DriveMode, DriveInstruction*> SingleDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new SingleModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_HOME, new DriveToHome()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_HOME_AT_START, new DriveHomeAtStart()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL_NAIVE, new DriveToBallNaive()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DIRVE_TO_BALL_AVOID_TURN, new DriveToBallAvoidTurn()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL_ANGLED, new DriveToBallAngled()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL_AIM_GATE, new DriveToBallAimGate()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_ROTATE_AROUND_BALL, new RotateAroundBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CRASH, new Crash()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_BORDER_TO_CLOSE, new BorderToClose()),
};

SingleModePlay::SingleModePlay(ICommunicationModule *pComModule, FieldState *pState)
		:StateMachine(pComModule, pState, TDriveModes(SingleDriveModes, SingleDriveModes + sizeof(SingleDriveModes) / sizeof(SingleDriveModes[0]))){};

