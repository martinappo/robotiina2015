#pragma once
#include "StateMachine.h"
enum MultiModeDriveStates {
	//2v2 modes
	DRIVEMODE_2V2_OFFENSIVE = 100,
	DRIVEMODE_2V2_DEFENSIVE,
	DRIVEMODE_2V2_KICKOFF,
	DRIVEMODE_2V2_AIM_GATE,
	DRIVEMODE_2V2_KICK,
	DRIVEMODE_2V2_DRIVE_TO_BALL,
	DRIVEMODE_2V2_CATCH_BALL,
	DRIVEMODE_2V2_DRIVE_HOME

};

class MultiModePlay :
	public StateMachine
{
private:
	bool isMaster;
public:
	MultiModePlay(ICommunicationModule *pComModule, FieldState *pState, bool bMaster);
	~MultiModePlay();
};

