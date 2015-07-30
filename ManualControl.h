#pragma once
#include "ConfigurableModule.h"
#include "ThreadedClass.h"

class ManualControl :
	public ConfigurableModule, public IControlModule, public ThreadedClass
{
public:
	ManualControl();
	virtual ~ManualControl();

	bool Init(ICommunicationModule *pComModule, FieldState *pState);
	void Run();

protected:
	ICommunicationModule *m_pComModule;
};

