#include "CoilBoard.h"
#include <chrono>
#include <thread>
#define TRIBBLER_QUEUE_SIZE 30
#define TRIBBLER_STATE_THRESHOLD 16

void CoilBoard::Kick(int force){
	boost::posix_time::ptime time2 = boost::posix_time::microsec_clock::local_time();
	//std::cout << (afterKickTime - time2).total_milliseconds() << std::endl;
	if ((time2 - afterKickTime).total_milliseconds() < 1500) return;
	//WriteString("k800\n");
	kick = true; // set flag, so that we do not corrupt writing in Run method
	//forcedNotInTribbler = true;
	afterKickTime = time2; //reset timer
	return;
}

void CoilBoard::ToggleTribbler(int speed){
	std::ostringstream oss;
	oss << ID_MAIN_BOARD << ":dm" << speed << "\n";
	if(m_pComPort) m_pComPort->WriteString(oss.str());
	
	return;
}

void CoilBoard::Run(){
	if(m_pComPort) m_pComPort->WriteString("5:c\n");
	boost::posix_time::time_duration::tick_type waitDuration;
	while (!stop_thread){
	try
	{
	
		//Pinging
		time = boost::posix_time::microsec_clock::local_time();
		waitDuration = (time - waitTime).total_milliseconds();
		if(m_pComPort) m_pComPort->WriteString("5:bl\n");
		if (waitDuration > 300){
			if(m_pComPort) m_pComPort->WriteString("5:p\n");
			waitTime = time;
		}
		if (kick) {
			std::cout << "kick ----->" << std::endl;
			if (m_pComPort) m_pComPort->WriteString("5:k\n");
			if (m_pComPort) m_pComPort->WriteString("5:c\n");
			kick = false;
		}
		/*
		//Forcing ballintribler false after kick
		boost::posix_time::time_duration::tick_type afterKickDuration = (time - afterKickTime).total_milliseconds();
		if (afterKickDuration > 1000 && forcedNotInTribbler){
			//forcedNotInTribbler = false;
		}
		else if (forcedNotInTribbler){
			ballInTribblerCount = -1;
		}
		*/
		std::chrono::milliseconds dura(10);
		std::this_thread::sleep_for(dura);
	}
	catch (...){
		std::cout << "Error writing or reading coilboard " << std::endl;
		stop_thread = true;
	}

	}
	try
	{
		std::ostringstream oss;
		oss << ID_MAIN_BOARD << ":dm" << 0 << "\n";
		if(m_pComPort) m_pComPort->WriteString(oss.str());
	}
	catch (...){
		std::cout << "Error writing or reading coliboard (try 2) " << std::endl;
		stop_thread = true;
	}

	std::cout << "CoilBoard stoping" << std::endl;
	
}