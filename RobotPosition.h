#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "GatePosition.h"

class RobotPosition : public ObjectPosition
{
public:
#ifdef WIN32
	RobotPosition();
#else
	RobotPosition() {}noexcept;
#endif
	RobotPosition(GatePosition yellowGate, GatePosition blueGate);
	RobotPosition(int x, int y);
	virtual ~RobotPosition();
	virtual void updatePolarCoords();
	virtual void updateFieldCoords(GatePosition yellowGate, GatePosition blueGate);
	virtual void updateCoordinates(GatePosition yellowGate, GatePosition blueGate);
private:
	void initPolarCoordinates();
};
