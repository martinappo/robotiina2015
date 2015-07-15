#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <atomic>
#include <boost/timer/timer.hpp>
#include "ThreadedClass.h"

class BasicWheel : public ThreadedClass
{
public:
	BasicWheel(const std::string &name);
	virtual ~BasicWheel();
	void SetSpeed(int given_speed);
	int GetSpeed() {
		return actual_speed;
	};
	double GetSpeedInRPM() {
		return actual_speed * 62.5 / (18.75 * 45) * 60;
	};
	int GetDistanceTraveled(bool reset = true);
	bool IsStalled() {
		return stall;
	}

protected:
	std::atomic_bool stall;
	//boost::mutex mutex;
	std::atomic_int target_speed;
	std::atomic_int actual_speed;
	int last_speed = 0;
    	std::atomic_bool update_speed;
	int id = 0;
        std::atomic_bool error;


	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
	boost::posix_time::ptime lastUpdate = time;
	boost::posix_time::ptime stallTime = time + boost::posix_time::seconds( 60 );
	boost::posix_time::time_duration stallDuration;
	long distance_traveled = 0;
	void CheckStall();
	virtual void UpdateSpeed() = 0;
	void Run();

};

class SoftwareWheel : public BasicWheel
{
public:
	SoftwareWheel(const std::string &name) : BasicWheel(name){}
protected:
	int max_acceleration = 1000;
	double stop_time = 1600;
	boost::posix_time::ptime stallStart;
	void UpdateSpeed();
};


class SerialWheel : public SimpleSerial, public BasicWheel
{
protected:
	void UpdateSpeed();
	int max_acceleration = 500;
public:
	SerialWheel(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200, const std::string &name="") : SimpleSerial(io_service, port, baud_rate), BasicWheel(name) {
	};
	virtual ~SerialWheel();

};
