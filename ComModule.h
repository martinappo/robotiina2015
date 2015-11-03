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
	virtual void Kick(int force = 800) {
		m_pCoilGun->Kick(force);
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
	std::string GetPlayCommand(){
		//throw std::runtime_error("Implement ComModule::GetPlayCommand");
		return "START";
	}

protected:
	IWheelController *m_pWheels;
	ICoilGun * m_pCoilGun;
};

