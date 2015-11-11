#include "CoilBoard.h"
#include <chrono>
#include <thread>
#define TRIBBLER_QUEUE_SIZE 30
#define TRIBBLER_STATE_THRESHOLD 16

void CoilBoard::Kick(int force){
	boost::posix_time::ptime time2 = boost::posix_time::microsec_clock::local_time();
	//std::cout << (afterKickTime - time2).total_milliseconds() << std::endl;
	if ((time2 - afterKickTime).total_milliseconds() < 1500) return;
	//writeString("k800\n");
	kick = true; // set flag, so that we do not corrupt writing in Run method
	//forcedNotInTribbler = true;
	afterKickTime = time2; //reset timer
	return;
}

void CoilBoard::ToggleTribbler(bool start){
	std::ostringstream oss;
	if (start) {
		oss << ID_MAIN_BOARD << ":dm" << 50 << "\n";
	}
	else{
		oss << ID_MAIN_BOARD << ":dm" << 0 << "\n";
	}
	m_pComPort->writeString(oss.str());
	
	return;
}

void CoilBoard::Run(){
	m_pComPort->writeString("c\n");
	boost::posix_time::time_duration::tick_type waitDuration;
	while (!stop_thread){
	try
	{
	
		//Pinging
		time = boost::posix_time::microsec_clock::local_time();
		waitDuration = (time - waitTime).total_milliseconds();
		m_pComPort->writeString("bl\n");
		std::string line = m_pComPort->readLineAsync(10);
		BallInTribbler = line == "<bl:1>";
		if (waitDuration > 300){
			m_pComPort->writeString("p\n");
			waitTime = time;
		}
		if (kick) {
			std::cout << "kick ----->" << std::endl;
			m_pComPort->writeString("k800\n");
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
		m_pComPort->writeString(oss.str());
	}
	catch (...){
		std::cout << "Error writing or reading coliboard (try 2) " << std::endl;
		stop_thread = true;
	}

	std::cout << "CoilBoard stoping" << std::endl;
	
}