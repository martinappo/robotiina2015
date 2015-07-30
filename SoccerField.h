#pragma once
#include "ThreadedClass.h"
#include "types.h"
#include <boost/thread/mutex.hpp>

class SoccerField :
	public ThreadedClass, public FieldState
{
public:
	SoccerField();
	virtual ~SoccerField();
	void Run();


};

