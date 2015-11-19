#pragma once
#include "ConfigurableModule.h"
#include "ThreadedClass.h"

class ManualControl :
	public ConfigurableModule, public IControlModule, public ThreadedClass
{
public:
	ManualControl(ICommunicationModule *pComModule);
	virtual ~ManualControl();

	void Run();
protected:
	ICommunicationModule *m_pComModule;
	// accelerations
	cv::Point2d speed = 0;
	double rotation = 0;
	boost::posix_time::ptime last_tick;
};

