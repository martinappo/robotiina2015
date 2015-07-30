#include "ComModule.h"


ComModule::ComModule(IWheelController * pWheels, ICoilGun *pCoilGun)
{
	m_pWheels = pWheels;
	m_pCoilGun = pCoilGun;

}


ComModule::~ComModule()
{
}
