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
	std::atomic_bool kickAllowed;
	std::atomic_int kickForce ;
	std::atomic_int ballInTribblerCount;
	std::mutex lock;
	ISerial *m_pComPort;
	//bool forcedNotInTribbler = false;
	int lastTribblerSpeed=0;
public:
	CoilBoard(ISerial *port) : m_pComPort(port), ThreadedClass("CoilBoard") {
		kickForce = 0;
		ballInTribbler = false;
		ballInTribblerCount = 0;
		kickAllowed = true;
		if (m_pComPort) {
			m_pComPort->SetMessageHandler(this);
		}
		ballCatchTime = boost::posix_time::microsec_clock::local_time();
		ballLostTime = boost::posix_time::microsec_clock::local_time();

		Start();
	};
	void Kick(int force);
	void ToggleTribbler(int speed);
	bool BallInTribbler(bool wait=false) { 
		if(wait) {
			if(ballInTribbler && BallInTribblerTime() > 300) {
				tribblerLastState = true;
			}
			else if(!ballInTribbler && BallNotInTribblerTime() > 100){
				tribblerLastState = false;
			};
			return tribblerLastState;
			
		}else {
			return ballInTribbler; 
		}
	}
	long BallInTribblerTime(); 
	long BallNotInTribblerTime();
	bool tribblerLastState = false;
	void Run();
	virtual ~CoilBoard() {
		if (m_pComPort) m_pComPort->SetMessageHandler(NULL);
		std::cout << "~CoilBoard" << std::endl;
		WaitForStop();
	}

	void DataReceived(const std::string & message);
	void HandleMessage(const std::string & message);

protected:
	std::atomic_bool ballInTribbler;
	boost::posix_time::ptime ballCatchTime;
	boost::posix_time::ptime ballLostTime;
	std::string last_message;


};
