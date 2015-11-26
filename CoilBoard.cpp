#include "CoilBoard.h"
#include <chrono>
#include <thread>
#define TRIBBLER_QUEUE_SIZE 30
#define TRIBBLER_STATE_THRESHOLD 16

void CoilBoard::DataReceived(const std::string & message)
{
	for (int i = 0; i < message.length(); i++){
		if (message[i] == '\n'){
			HandleMessage(last_message);
			last_message = "";
		}
		else 
			last_message += message[i];
	}
}
void CoilBoard::HandleMessage(const std::string & message)
{
	//<5:bl:0>
	if ((message[1] - '0') == ID_MAIN_BOARD){
		
		if ((message[3] == 'b') && (message[4] == 'l')){
			ballInTribbler = (message[6] == '1');
		}
		else if (message[3] == 'c') { // charge done
			//TODO: allow kicking again, instead of waiting fo 1.5 sec below
		}
	}

}


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
	if(m_pComPort) m_pComPort->WriteString("5:dm0\n");
	if(m_pComPort) m_pComPort->WriteString("5:fs0\n");
	if(m_pComPort) m_pComPort->WriteString("5:c\n");
	boost::posix_time::time_duration::tick_type waitDuration;
	while (!stop_thread){
		try
		{
		
			//Pinging
			time = boost::posix_time::microsec_clock::local_time();
			waitDuration = (time - waitTime).total_milliseconds();
			//if(m_pComPort) m_pComPort->WriteString("5:bl\n");
			if (waitDuration > 300){
				if(m_pComPort) m_pComPort->WriteString("5:p\n");
				waitTime = time;
			}
			if (kick) {
				std::cout << "kick ----->" << std::endl;
				if (m_pComPort) m_pComPort->WriteString("5:k\n");
				Sleep(100);
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
			;
			Sleep(50);
		}
		catch (...){
			std::cout << "Error writing or reading coilboard " << std::endl;
			stop_thread = true;
		}

	}
			std::cout << "Coilboard stoping " << std::endl;
	
	try
	{
		std::ostringstream oss;

		if (m_pComPort) m_pComPort->SendCommand(ID_MAIN_BOARD, "fs", 1);
		if (m_pComPort) m_pComPort->SendCommand(ID_MAIN_BOARD, "dm", 0);
		/*
		for(int i = 0; i< 20; i++) {
			if (m_pComPort) m_pComPort->SendCommand(ID_MAIN_BOARD, "k", 800 + (i * 50));
			Sleep(200);
		}
		Sleep(1000);
		*/
	}
	catch (...){
		std::cout << "Error writing or reading coliboard (try 2) " << std::endl;
		stop_thread = true;
	}

	std::cout << "CoilBoard stoping" << std::endl;
	
}