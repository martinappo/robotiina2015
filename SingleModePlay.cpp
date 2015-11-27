#include "SingleModePlay.h"
#include "AutoPlayHelpers.h"

/*BEGIN DriveToBall*/
void DriveToBall::onEnter()
{
	DriveInstruction::onEnter();	
}

DriveMode DriveToBall::step(double dt){
	//return stepNaive(dt);
	if (STUCK_IN_STATE(500)) m_pCom->ToggleTribbler(0);
	return stepAngled(dt);
	//return stepPenatalizeRotation(dt);
}
DriveMode DriveToBall::stepNaive(double dt)
{
	
	auto &target = getClosestBall();
	if (target.getDistance() > 10000) return DRIVEMODE_IDLE;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;
	//std::cout << std::endl << "aimtarget0, " ;

	if (aimTarget(target,10)){
		if (driveToTarget(target)){
			if (aimTarget(target,1.5)){
				return DRIVEMODE_CATCH_BALL;
			}
		}
	}
	return DRIVEMODE_DRIVE_TO_BALL;
} 
DriveMode DriveToBall::stepAngled(double dt)
{
	auto &target = getClosestBall();
	if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;
	if (driveToTargetWithAngle(target, 25, 5)){
		return DRIVEMODE_CATCH_BALL;
	}
	else {
		return DRIVEMODE_DRIVE_TO_BALL;
	}
		

}
DriveMode DriveToBall::stepPenatalizeRotation(double dt)
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
	if (aimTarget(target, 10)){
		if (driveToTarget(target)){
			if (aimTarget(target, 2)){
				return DRIVEMODE_CATCH_BALL;
			}
		}
	}
	return DRIVEMODE_DRIVE_TO_BALL;

}

class DriveToHome : public DriveInstruction
{
public:
	DriveToHome(const std::string &name = "DRIVE_HOME") : DriveInstruction(name){};
	virtual DriveMode step(double dt){
		auto target = m_pFieldState->GetHomeGate();
		if (target.getDistance() < 50) {
			return DRIVEMODE_DRIVE_TO_BALL;
		}
		else {
			m_pCom->Drive(40, -target.getHeading());
		}
	return DRIVEMODE_DRIVE_HOME;
	}

};



/*BEGIN CatchBall*/
void CatchBall::onEnter()
{
	DriveInstruction::onEnter();
	m_pCom->ToggleTribbler(100);
	FIND_TARGET_BALL
	initDist = target.getDistance();
	STOP_DRIVING
}
DriveMode CatchBall::step(double dt)
{
	FIND_TARGET_BALL //TODO: use it?
	if (STUCK_IN_STATE(3000) || target.getDistance() > initDist  + 10) return DRIVEMODE_DRIVE_TO_BALL;
	if(catchTarget(target)) { 
		return DRIVEMODE_AIM_GATE;
	}
	return DRIVEMODE_CATCH_BALL;
}
void CatchBall::onExit()
{
	//DO_NOT_STOP_TRIBBLER
}
/*END CatchBall*/


/*BEGIN AimGate*/

DriveMode AimGate::step(double dt)
{
	FIND_TARGET_GATE
	if (!BALL_IN_TRIBBLER) return DRIVEMODE_DRIVE_TO_BALL;	
	double errorMargin;
	if (target.getDistance() > 200){
		errorMargin = 10;
	}
	else errorMargin = 15;
	if (aimTarget(target, errorMargin)){
		return DRIVEMODE_KICK;
	}
	return DRIVEMODE_AIM_GATE;
}


/*BEGIN Kick*/
void Kick::onEnter()
{
	DriveInstruction::onEnter();
	STOP_TRIBBLER
	STOP_DRIVING
}
DriveMode Kick::step(double dt)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	m_pCom->Kick();
	std::this_thread::sleep_for(std::chrono::milliseconds(50)); //less than half second wait.
	return DRIVEMODE_DRIVE_TO_BALL;
}


std::pair<DriveMode, DriveInstruction*> SingleDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new SingleModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_HOME, new DriveToHome()),
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

