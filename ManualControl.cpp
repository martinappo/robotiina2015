#include "ManualControl.h"
#include <chrono>
#include <thread>

ManualControl::ManualControl(ICommunicationModule *pComModule) :ConfigurableModule("ManualControl"), ThreadedClass("ManualControl")
{
	speed = { 0, 0 };
	rotation = 0;

	m_pComModule = pComModule;

	AddSetting("Turn Left", []{return "r"; }, [this] {this->rotation -= 10; });
	AddSetting("Turn Right", []{return "l"; }, [this]{this->rotation += 10; });

	AddSetting("Move Left", []{return "a"; }, [this] {this->speed.x -= 10; });
	AddSetting("Move Right", []{return "d"; }, [this]{this->speed.x += 10; });

	AddSetting("Move Forward", []{return "w"; }, [this]{this->speed.y += 10; });
	AddSetting("Move Back", []{return "s"; }, [this]{this->speed.y -= 10; });
	AddSetting("Stop (space)", []{return "q"; }, [this]{this->speed = cv::Point2d(0,0); });

//	AddSetting("Rotate Right", []{return ""; }, [this]{this->wheels->Rotate(0, 20); });
//	AddSetting("Rotate Left", []{return ""; }, [this]{this->wheels->Rotate(1, 20); });
	AddSetting("Kick", []{return "k"; }, [this] {this->m_pComModule->Kick(); });
	AddSetting("Start tribbler", []{return "z"; }, [this]{this->m_pComModule->ToggleTribbler(100); });
	AddSetting("Stop tribbler", []{return "x"; }, [this]{this->m_pComModule->ToggleTribbler(0); });
	//Start();
}


void ManualControl::Run(){
	speed = { 0, 0 };
	rotation = 0;
	while (!stop_thread){
		last_tick = boost::posix_time::microsec_clock::local_time();
		m_pComModule->Drive(speed, rotation);
		boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
		double dt = (double)((time - last_tick).total_milliseconds())/1000;

		rotation -= sign0(rotation);
		speed.x -= sign0(speed.x)*dt;
		speed.y -= sign0(speed.y)*dt;
		last_tick = time;
		Sleep(100);
	}
}

ManualControl::~ManualControl()
{
}
