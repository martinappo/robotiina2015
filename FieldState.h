#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "BallPosition.h"
#include "GatePosition.h"
#include "RobotPosition.h"
#include "TargetPosition.h"

class FieldState {
public:
	FieldState();
	virtual ~FieldState();
	BallPosition balls[NUMBER_OF_BALLS]; //All others are distance from self and heading to it
	GatePosition blueGate;
	GatePosition yellowGate;
	RobotPosition self; //Robot distance on field
	std::atomic_bool gateObstructed;
//	virtual void SetTargetGate(OBJECT gate) = 0;
//	virtual GatePosition &GetTargetGate() = 0;
	void resetBallsUpdateState();
	virtual void Lock() {};
	virtual void UnLock() {};
};

class FieldStateLock{
public:
	FieldStateLock(FieldState * pFieldState){
		m_pState = pFieldState;
		m_pState->Lock();
	}
	~FieldStateLock(){
		m_pState->UnLock();
	}
	FieldState * operator ->(){
		return m_pState;
	}
private:
	FieldState * m_pState;
};