#pragma  once
#include "types.h"
#include "SimpleSerial.h"
#include <atomic>
#include <boost/timer/timer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ThreadedClass.h"

class CoilGun: public ICoilGun
{
protected:
	std::atomic_bool ballInTribbler;
public:
	CoilGun() {
		ballInTribbler = false;
	}
	virtual void ToggleTribbler(bool start){};
	virtual bool BallInTribbler(){ 
		if (rand() % 100 > 95){
			ballInTribbler = !ballInTribbler;
		};
		return ballInTribbler;
	};
	virtual void Kick() {
		ballInTribbler = false;
	};
	virtual ~CoilGun() {
		std::cout << "~CoilGun" << std::endl;
	}
	
};

class CoilBoard : public CoilGun, public SimpleSerial, public ThreadedClass
{
private:
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime waitTime = time;
	boost::posix_time::time_duration waitDuration;
	boost::posix_time::ptime afterKickTime = time;
	//boost::posix_time::time_duration afterKickDuration;
	std::atomic_bool kick;
	std::atomic_int ballInTribblerCount;
	//bool forcedNotInTribbler = false;

public:
	CoilBoard(boost::asio::io_service &io_service, std::string port = "", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate), ThreadedClass("CoilBoard") {
		ballInTribbler = false;
		ballInTribblerCount = 0;
		kick = false;
		Start();
	};
	void Kick();
	void ToggleTribbler(bool start);
	bool BallInTribbler();
	void Run();
	virtual ~CoilBoard() {
		std::cout << "~CoilBoard" << std::endl;
		WaitForStop();
	}

};











