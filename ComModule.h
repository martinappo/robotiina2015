#pragma once
#include "types.h"
class ComModule :
	public ICommunicationModule
{
public:
	ComModule(IWheelController * pWheels, ICoilGun *pCoilGun);
	void Init() {};
	virtual ~ComModule();

	virtual void Drive(double fowardSpeed, double direction=0, double angularSpeed=0) {
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
	const Speed & GetActualSpeed(){
		return m_pWheels->GetActualSpeed();
	}
	const Speed & GetTargetSpeed(){
		return m_pWheels->GetTargetSpeed();
	}
	std::string GetDebugInfo() { return ""; }
	bool IsReal(){
		return true;
	}

protected:
	IWheelController *m_pWheels;
	ICoilGun * m_pCoilGun;
};

