#pragma once
#include "types.h"
#include "CoilBoard.h"
#include "WheelController.h"

class ComModule :
	public ICommunicationModule
{
public:
	ComModule(ISerial *pSerialPort);
	virtual ~ComModule();

	virtual void Drive(double fowardSpeed, double direction=0, double angularSpeed=0) {
		m_pWheels->Drive(fowardSpeed, direction, angularSpeed);
	};
	virtual void Drive(const cv::Point2d &speed, double angularSpeed = 0){
		m_pWheels->Drive(speed, angularSpeed);
	}

	virtual bool BallInTribbler(bool wait=false) {
		return m_pCoilGun->BallInTribbler(wait);
	}

	virtual long BallInTribblerTime() {
		return m_pCoilGun->BallInTribblerTime();
	}
	virtual void Kick(int kick) {
		m_pCoilGun->Kick(kick);
	}
	virtual void ToggleTribbler(int speed){
		m_pCoilGun->ToggleTribbler(speed);
	}
	const Speed & GetActualSpeed(){
		return m_pWheels->GetActualSpeed();
	}
	const Speed & GetTargetSpeed(){
		return m_pWheels->GetTargetSpeed();
	}
	std::string GetDebugInfo() { return m_pWheels->GetDebugInfo() + "\n"; }
	bool IsReal(){
		return true;
	}

protected:
	WheelController *m_pWheels;
	CoilBoard * m_pCoilGun;
};

