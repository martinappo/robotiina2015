#include "ComModule.h"

ComModule::ComModule(ISerial *pSerialPort)
{
	m_pWheels = new WheelController(pSerialPort, 4);
	m_pCoilGun = new CoilBoard(pSerialPort);

}


ComModule::~ComModule()
{
	delete m_pWheels;
	delete m_pCoilGun;
}
