#include "CoilBoard.h"
#include <chrono>
#include <thread>
#define TRIBBLER_QUEUE_SIZE 30
#define TRIBBLER_STATE_THRESHOLD 16

void CoilBoard::Kick(){
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
	if (start) {
		writeString("m1\n");
	}
	else{
		writeString("m0\n");
	}
	
	return;
}


bool CoilBoard::BallInTribbler(){

	return ballInTribblerCount > 0;
}

void CoilBoard::Run(){
	writeString("c\n");
	boost::posix_time::time_duration::tick_type waitDuration;
	while (!stop_thread){
	try
	{
	
		//Pinging
		time = boost::posix_time::microsec_clock::local_time();
		waitDuration = (time - waitTime).total_milliseconds();
		std::string line = readLineAsync(10);
		if(line == "true" || line == "false"/* && !forcedNotInTribbler*/){
			//std::cout << "ballInTribblerCount " << ballInTribblerCount << " " << line << std::endl;
			int newcount = ballInTribblerCount + ((line == "true") ? 1 : -1);
			//std::cout << "ballInTribblerCount " << ballInTribblerCount << " " << newcount << " " << line << std::endl;
			ballInTribblerCount = std::min(2, std::max(-2, newcount));
 		}
		if (waitDuration > 300){
			writeString("p\n");
			waitTime = time;
			//std::cout << "ping " << waitDuration << std::endl;
		} else {
			writeString("b\n");
		}
		if (kick) {
			std::cout << "kick ----->" << std::endl;
			writeString("k800\n");
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

	writeString("d\n");
	writeString("m0\n");
	}
	catch (...){
		std::cout << "Error writing or reading coliboard (try 2) " << std::endl;
		stop_thread = true;
	}

	std::cout << "CoilBoard stoping" << std::endl;
	
}




















