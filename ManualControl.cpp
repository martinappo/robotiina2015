#include "ManualControl.h"
#include <chrono>
#include <thread>

ManualControl::ManualControl() :ConfigurableModule("ManualControl")
{

	AddSetting("Move Left", []{return "a"; }, [this] {this->m_pComModule->Drive(40, 90, 0); });
	AddSetting("Move Right", []{return "d"; }, [this]{this->m_pComModule->Drive(40, -90, 0); });
	AddSetting("Move Forward", []{return "w"; }, [this]{this->m_pComModule->Drive(190, 0, 0); });
	AddSetting("Move Back", []{return "s"; }, [this]{this->m_pComModule->Drive(-40, 0, 0); });
//	AddSetting("Rotate Right", []{return ""; }, [this]{this->wheels->Rotate(0, 20); });
//	AddSetting("Rotate Left", []{return ""; }, [this]{this->wheels->Rotate(1, 20); });
	AddSetting("Kick", []{return " "; }, [this] {this->m_pComModule->Kick(); });
	AddSetting("Start tribbler", []{return "z"; }, [this]{this->m_pComModule->ToggleTribbler(true); });
	AddSetting("Stop tribbler", []{return "x"; }, [this]{this->m_pComModule->ToggleTribbler(false); });

}
bool ManualControl::Init(ICommunicationModule *pComModule, FieldState *pState){
	m_pComModule = pComModule;
	return true;
}

void ManualControl::Run(){
	while (!stop_thread){
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

ManualControl::~ManualControl()
{
}
