#include "SingleModePlay.h"
#include "AutoPlayHelpers.h"

DriveMode DriveToBall::step(double dt)
{
	auto &target = getClosestBall();
	if (target.getDistance() > 10000) return DRIVEMODE_IDLE;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;
	std::cout << std::endl << "aimtarget0, " ;
	if (aimTarget(target)){
		if (driveToTarget(target)){
			if (aimTarget(target, 5)){
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
	m_pCom->ToggleTribbler(100);
	STOP_DRIVING
}
void CatchBall::onExit(){ //DO_NOT_STOP_TRIBBLER
}

DriveMode CatchBall::step(double dt)
{
	FIND_TARGET_BALL

	if (STUCK_IN_STATE(3000)) return DRIVEMODE_DRIVE_TO_BALL;
	if(catchTarget(target)) {
		return DRIVEMODE_AIM_GATE;
	}
	return DRIVEMODE_CATCH_BALL;
}

/*END CatchBall*/


/*BEGIN AimGate*/
DriveMode AimGate::step(double dt)
{

	FIND_TARGET_GATE
	if (!BALL_IN_TRIBBLER) return DRIVEMODE_DRIVE_TO_BALL;	
	double errorMargin;
	if (target.getDistance() > 200){
		errorMargin = 5;
	}
	else errorMargin = 10;
	if (aimTarget(target, errorMargin)){
		return DRIVEMODE_KICK;
	}
	return DRIVEMODE_AIM_GATE;

}


/*BEGIN Kick*/
DriveMode Kick::step(double dt)
{
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

