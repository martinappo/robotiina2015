#pragma  once
#include "types.h"
#include <atomic>
#include <boost/timer/timer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ThreadedClass.h"

class CoilBoard : public ThreadedClass, public ISerialListener
{
private:
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime waitTime = time;
	boost::posix_time::time_duration waitDuration;
	boost::posix_time::ptime afterKickTime = time;
	//boost::posix_time::time_duration afterKickDuration;
	std::atomic_bool kick;
	std::atomic_int ballInTribblerCount;
	ISerial *m_pComPort;
	//bool forcedNotInTribbler = false;

public:
	CoilBoard(ISerial *port) : m_pComPort(port), ThreadedClass("CoilBoard") {
		ballInTribbler = false;
		ballInTribblerCount = 0;
		kick = false;
		if (m_pComPort) {
			m_pComPort->SetMessageHandler(this);
		}
		Start();
	};
	void Kick(int force = 800);
	void ToggleTribbler(int speed);
	bool BallInTribbler() { return ballInTribbler; }
	void Run();
	virtual ~CoilBoard() {
		if (m_pComPort) m_pComPort->SetMessageHandler(NULL);
		std::cout << "~CoilBoard" << std::endl;
		WaitForStop();
	}

	void DataReceived(const std::string & message){
		if (message.substr(0, 4) == "<bl:")
		ballInTribbler = message.substr(4, 1) == "1";
	}
protected:
	std::atomic_bool ballInTribbler;


};