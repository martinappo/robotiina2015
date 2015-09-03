#include "Wheel.h"
#include <chrono>
#include <thread>

BasicWheel::BasicWheel(const std::string &name): ThreadedClass("wheel "+ name)
{
	stall = false;
	update_speed = false;
	target_speed = 0;
	actual_speed = 0;
error = false;
}


void BasicWheel::SetSpeed(int given_speed) {
	//boost::mutex::scoped_lock lock(mutex);
	target_speed = given_speed;
	update_speed = true;
};

void BasicWheel::Run()
{
	while (!stop_thread){
		time = boost::posix_time::microsec_clock::local_time();
		{ // new scope for locking
			//boost::mutex::scoped_lock lock(mutex);
			UpdateSpeed();
			//CheckStall();
			//CalculateDistanceTraveled();
		}
		lastStep = time;
		// speed update interval is 62.5Hz
		std::this_thread::sleep_for(std::chrono::milliseconds(30)); // do not poll serial to fast

	}
	std::cout << "Wheel stoping" << std::endl;
}

void BasicWheel::CheckStall()
{
	if (target_speed == 0) {
		stall = false;
		return;
	}

	double diff = abs(actual_speed - target_speed);
	double actual_p = ((double)actual_speed / (double)target_speed) * 100; // how much we have reached from target speed (in %)
	
	boost::posix_time::time_duration::tick_type stallDuration = (time - stallTime).total_milliseconds();
	if (abs(actual_p) < 45) { 
		if (!stall && stallDuration > 900){ 
			std::cout << "stalled, diff: " << actual_p << "%, " << diff << " a = " << actual_speed << " != t " << target_speed << ", dt: " << stallDuration << std::endl;
			stall = true; 
		}
	}
	else{
		stallTime = time;
		stall = false;
	}
			std::cout << "check, diff: " << actual_p << "%, " << diff << " a = " << actual_speed << " != t " << target_speed << "dt: " << stallDuration << std::endl;

}

int BasicWheel::GetDistanceTraveled(bool reset) 
{
	{ // new scope for locking
		//boost::mutex::scoped_lock lock(mutex);
		return distance_traveled;
		if (reset) distance_traveled = 0;
	}

}

BasicWheel::~BasicWheel()
{
	WaitForStop();
}

void SoftwareWheel::UpdateSpeed()
{
	last_speed = actual_speed;
	double dt = (double)(time - lastStep).total_milliseconds() / 1000.0;
	if (dt < 0.0000001) return;

	if ((time - lastUpdate).total_milliseconds() > stop_time) { // die out if no update
		actual_speed = 0;
		return;
	}

	double dv = target_speed - actual_speed;
	double sign = (dv > 0) - (dv < 0);
	double acc = dv / dt;
	acc = sign * std::min((int)abs(acc), max_acceleration);

	if (false && rand() % 1000 > 995) { // 0.5% probability to stall
		stallStart = time;
		actual_speed = 0;
	}
	else if ((time - stallStart).total_milliseconds() < 600) {
		;// still stalled
	}
	else {
		actual_speed += (int)(acc * dt);
	}
};

void SerialWheel::UpdateSpeed()
{
	try
	{

		if (update_speed){
			lastUpdate = boost::posix_time::microsec_clock::local_time();
			std::ostringstream oss;
			oss << "sd" << target_speed << "\n";
			writeString(oss.str());
			update_speed = false;
		}
		writeString("s\n");
		std::string line = readLine();
		if (line.length() > 2)
			actual_speed = atoi(line.substr(3).c_str());
		else
			actual_speed = 0;
	}
	catch(std::exception const&  ex)
	{
		std::cout << "Error writing or reading wheel speed: " << ex.what() << std::endl;
		stop_thread = true;
		error = true;
	}
	catch (...){
		std::cout << "Error writing or reading wheel speed " << std::endl;
		stop_thread = true;
		error = true;
	}
};

	SerialWheel::~SerialWheel(){
	std::cout << "~SerialWheel 1" << std::endl;
	WaitForStop();
	std::cout << "~SerialWheel 2" << std::endl;
	if (!error) {
		SetSpeed(0);
		UpdateSpeed();
	}
}
