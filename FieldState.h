#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "BallPosition.h"

class FieldState {
public:
	std::atomic<ObjectPosition> self; // our robot distance from center and rotation
	std::atomic<BallPosition> balls[NUMBER_OF_BALLS];// all others are distance from self and heading to it
	std::atomic<ObjectPosition> blueGate;
	std::atomic<ObjectPosition> yellowGate;
	std::atomic_bool gateObstructed;
	virtual void SetTargetGate(OBJECT gate) = 0;
	virtual ObjectPosition GetTargetGate() const = 0;
	void resetBallsUpdateState();
};