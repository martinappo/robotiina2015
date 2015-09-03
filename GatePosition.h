#pragma once
#include "types.h"
#include "ObjectPosition.h"

class GatePosition : public ObjectPosition
{
public:
#ifdef WIN32
	GatePosition() {};
#else
	GatePosition() noexcept {};
#endif
	GatePosition(int x, int y); //Values relative to field and in cm
	GatePosition(OBJECT gate);
	virtual ~GatePosition();
	int id;

	virtual void updateFieldCoords();
	virtual void updateCoordinates(int x, int y);
	virtual void updateCoordinates(cv::Point rawCoords);

};