#include "SingleModePlay.h"
#include "AutoPlayHelpers.h"

DriveMode DriveToBall::step(double dt)
{
	auto &target = getClosestBall();
	if (target.getDistance() > 10000) return DRIVEMODE_IDLE;
	if (m_pCom->BallInTribbler()) return DRIVEMODE_AIM_GATE;
	std::cout << std::endl << "aimtarget0, " ;
	if (aimTarget(target)){
		std::cout << "aimTarget1, ";
		if (driveToTarget(target)){
			std::cout << "driveToTarget, ";
			if (aimTarget(target)){
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


	if (!BALL_IN_TRIBBLER) return DRIVEMODE_DRIVE_TO_BALL;

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

