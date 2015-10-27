#include "SingleModePlay.h"
#include "AutoPlayHelpers.h"

DriveMode DriveToBall::step(double dt)
{
	CHECK_FOR_STOP
	if (BALL_IN_TRIBBLER) return DRIVEMODE_AIM_GATE;
	FIND_TARGET_BALL
	if (TARGET_BALL_NOT_FOUND) return DRIVEMODE_DRIVE_HOME;

	if (TARGET_BALL_TOO_FAR) return DRIVEMODE_IDLE;
	else if (TARGET_BALL_IS_VERY_CLOSE){
		if(TARGET_BALL_IS_IN_CENTER) return DRIVEMODE_CATCH_BALL;
		else {
			ROTATE_TOWARD_TO_TARGET;
			START_TRIBBLER;
		}
	}
	else {
		ROTATE_AND_DRIVE_TOWARD_TO_TARGET;
		STOP_TRIBBLER;
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
	CHECK_FOR_STOP

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
	if (TARGET_GATE_IS_IN_CENTER) {
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