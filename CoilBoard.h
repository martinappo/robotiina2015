#pragma  once
#include "types.h"
#include <atomic>
#include <boost/timer/timer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ThreadedClass.h"
#include <mutex>

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
	std::mutex lock;
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
		std::lock_guard<std::mutex> lockMe(lock);
		if (message.empty()) return;
		std::string m = last_message + message;
		size_t eol = m.rfind('\n');
		if (eol == std::string::npos){
			// no end-of-line found, store it for future
			last_message = m;
		}
		else if (m.length() > 3 ){
			ballInTribbler = m.substr(eol-2, 1) == "1";
			last_message = m.substr(eol+1);
		} else {
			last_message = m;
		}

	}
protected:
	std::atomic_bool ballInTribbler;
	std::string last_message;


};