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
};

