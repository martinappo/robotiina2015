#include "ManualControl.h"
#include <chrono>
#include <thread>

ManualControl::ManualControl(ICommunicationModule *pComModule) :ConfigurableModule("ManualControl")
{
	m_pComModule = pComModule;

	AddSetting("Turn Left", []{return "a"; }, [this] {this->rotation -= 3; });
	AddSetting("Turn Right", []{return "d"; }, [this]{this->rotation += 3; });

	AddSetting("Move Left", []{return "A"; }, [this] {this->speed += 3; this->direction = -90; });
	AddSetting("Move Right", []{return "D"; }, [this]{this->speed += 3; this->direction = 90; });

	AddSetting("Move Forward", []{return "w"; }, [this]{this->speed += 13; });
	AddSetting("Move Back", []{return "s"; }, [this]{this->speed -= 13; });
	AddSetting("Stop (space)", []{return " "; }, [this]{this->speed = 0; });

//	AddSetting("Rotate Right", []{return ""; }, [this]{this->wheels->Rotate(0, 20); });
//	AddSetting("Rotate Left", []{return ""; }, [this]{this->wheels->Rotate(1, 20); });
	AddSetting("Kick", []{return " "; }, [this] {this->m_pComModule->Kick(); });
	AddSetting("Start tribbler", []{return "z"; }, [this]{this->m_pComModule->ToggleTribbler(50); });
	AddSetting("Stop tribbler", []{return "x"; }, [this]{this->m_pComModule->ToggleTribbler(0); });
	//Start();
}


void ManualControl::Run(){
	while (!stop_thread){
		m_pComModule->Drive(speed, direction, rotation);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
		if ((time - last_tick).total_milliseconds() > 100){
			rotation -= sign(rotation);
			speed -= sign(speed);
			last_tick = time;
			if (abs(speed) < 1) direction = 0;
		}
	}
}

ManualControl::~ManualControl()
{
}
