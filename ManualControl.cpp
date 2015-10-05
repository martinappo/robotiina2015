#include "ManualControl.h"
#include <chrono>
#include <thread>

ManualControl::ManualControl(ICommunicationModule *pComModule) :ConfigurableModule("ManualControl")
{
	m_pComModule = pComModule;

	AddSetting("Turn Left", []{return "a"; }, [this] {this->rotation += 10; });
	AddSetting("Turn Right", []{return "d"; }, [this]{this->rotation -= 10; });

	AddSetting("Move Left", []{return "A"; }, [this] {this->direction += 10; });
	AddSetting("Move Right", []{return "D"; }, [this]{this->direction -= 10; });

	AddSetting("Move Forward", []{return "w"; }, [this]{this->speed += 10; });
	AddSetting("Move Back", []{return "s"; }, [this]{this->speed -= 10; });

//	AddSetting("Rotate Right", []{return ""; }, [this]{this->wheels->Rotate(0, 20); });
//	AddSetting("Rotate Left", []{return ""; }, [this]{this->wheels->Rotate(1, 20); });
	AddSetting("Kick", []{return " "; }, [this] {this->m_pComModule->Kick(); });
	AddSetting("Start tribbler", []{return "z"; }, [this]{this->m_pComModule->ToggleTribbler(true); });
	AddSetting("Stop tribbler", []{return "x"; }, [this]{this->m_pComModule->ToggleTribbler(false); });
	//Start();
}


void ManualControl::Run(){
	while (!stop_thread){
		m_pComModule->Drive(speed, direction, rotation);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
		if ((time - last_tick).total_milliseconds() > 1000){
			rotation -= sign(rotation);
			direction -= sign(direction);
			speed -= sign(speed);
			last_tick = time;
		}
	}
}

ManualControl::~ManualControl()
{
}
