#pragma once
#include "types.h"
class ControlModule :
	public ICommunicationModule
{
public:
	ControlModule();
	virtual bool Init(IWheelController * pWheels, ICoilGun *pCoilGun) {
		m_pWheels = pWheels;
		m_pCoilGun = pCoilGun;
		return true;
	}

	virtual ~ControlModule();

	virtual void Drive(double fowardSpeed, double direction, double angularSpeed) {
		m_pWheels->Drive(fowardSpeed, direction, angularSpeed);
	};
	virtual bool BallInTribbler() {
		return m_pCoilGun->BallInTribbler();
	}
	virtual void Kick() {
		m_pCoilGun->Kick();
	}
	virtual void ToggleTribbler(bool start){
		m_pCoilGun->ToggleTribbler(start);
	}

protected:
	IWheelController *m_pWheels;
	ICoilGun * m_pCoilGun;
};

