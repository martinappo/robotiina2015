#pragma once
#include "StateMachine.h"
enum MultiModeDriveStates {
	DRIVEMODE_DEFEND = 100

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

