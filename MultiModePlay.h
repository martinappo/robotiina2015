#pragma once
#include "StateMachine.h"

class MultiModePlay :
	public StateMachine
{
private:
	bool isMaster;
public:
	MultiModePlay(ICommunicationModule *pComModule, FieldState *pState, bool bMaster);
	~MultiModePlay();
};

