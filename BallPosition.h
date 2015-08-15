#pragma once
#include "types.h"
#include "ObjectPosition.h"

class BallPosition : public ObjectPosition
{
public:
	BallPosition();
	virtual ~BallPosition();
	int id;
	bool isValid = true;
};