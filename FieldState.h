#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "BallPosition.h"
#include "GatePosition.h"
#include "RobotPosition.h"

class FieldState {
public:
	RobotPosition self; //Robot distance on field
	BallPosition balls[NUMBER_OF_BALLS]; //All others are distance from self and heading to it
	GatePosition blueGate;
	GatePosition yellowGate;
	std::atomic_bool gateObstructed;
	virtual void SetTargetGate(OBJECT gate) = 0;
	virtual ObjectPosition GetTargetGate() const = 0;
	void resetBallsUpdateState();
};