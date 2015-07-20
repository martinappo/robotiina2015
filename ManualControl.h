#pragma once
#include "ConfigurableModule.h"
class ManualControl :
	public ConfigurableModule, public IControlModule
{
public:
	ManualControl();
	virtual ~ManualControl();

	virtual bool Init(IWheelController * pWheels, ICoilGun *pCoilGun) {
		wheels = pWheels;
		coilBoard = pCoilGun;
		return true;
	}

protected:
	IWheelController *wheels;
	ICoilGun * coilBoard;

};

