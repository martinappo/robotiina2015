#pragma once
#include "types.h"
#include "ThreadedClass.h"
#include "simpleserial.h"
#include <boost/atomic.hpp>


class Arduino
{
protected:
	cv::Point3i sonars = { 100, 100, 100 };
	IButtonClickListener *buttonListener = NULL;
	bool start = 0;
	bool old_start = 0;
	bool gte = -1;
public:
	Arduino(){}
	virtual ~Arduino(){}
	void Run(){};
	const cv::Point3i &GetSonarReadings(){ return sonars; }
	int getStart() { 
		int tmp = start; 
		start=0; 
		return tmp;
	};
	int getGate() { return gte;  };
	virtual bool BallInTribbler(){ throw std::runtime_error("Not implemented"); };
	std::string GetDebugInfo();

};


class ArduinoBoard : public Arduino, SimpleSerial, ThreadedClass
{
private:
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::atomic<bool> ballInTribbler;

public:
	static Arduino Create();
	ArduinoBoard(boost::asio::io_service &io_service, std::string port = "", unsigned int baud_rate = 115200)  : ThreadedClass("Arduino"), SimpleSerial(io_service, port, baud_rate) {
		stop_thread = false;
		ballInTribbler = false;
		Start();
	};
	virtual ~ArduinoBoard(){
		WaitForStop();
	}
	void Run();
	void Run2();

};











