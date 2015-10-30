#include "Simulator.h"
#include "DistanceCalculator.h"
#include <chrono>
#include <thread>
#include <time.h>       /* time */
extern DistanceCalculator gDistanceCalculator;
extern boost::asio::ip::address addr;

Simulator::Simulator(boost::asio::io_service &io, bool master) :ThreadedClass("Simulator"), UdpServer(io, addr, 31000, master), isMaster(master)
{
	srand(::time(NULL));

	self.fieldCoords = cv::Point(rand() % 300 - 150, rand() % 460 - 230);
	self.polarMetricCoords = cv::Point(0, 0);
	if (isMaster) {
		// distribute balls uniformly at random
		for (int i = 0; i < NUMBER_OF_BALLS; i++) {
			balls[i].fieldCoords.x = (int)(((i % 3) - 1) * 100) + rand() % 50;
			balls[i].fieldCoords.y = (int)((i / 3 - 1.5) * 110) + rand() % 50;
			balls[i].id = i;
		}
	}
	else {
		SendMessage("ID? X"); // ask slave id
	};
	Start();
}
void Simulator::MessageReceived(const std::string & message){
	std::stringstream ss(message);
	std::string command, id;
	ss >> command;
	if (isMaster) {
		if (command == "ID?") {
			SendMessage("ID= " + std::to_string(next_id++));
		}
		else if (command == "POS") { // foward to slaves
			SendMessage(message);
		}
	}
	else { // slave commands
		if (command == "ID=") {
			ss >> id;
			this->id = atoi(id.c_str());
		}
		else if (command == "BAL") {
			ss >> id;
			int _id = atoi(id.c_str());
			if (_id != this->id) {
				std::string x, y, a;
				ss >> x >> y;
				balls[_id].fieldCoords.x = atoi(x.c_str());
				balls[_id].fieldCoords.y = atoi(y.c_str());
			}
		}

	}
	if (command == "POS") {
		ss >> id;
		int _id = atoi(id.c_str());
		if (_id != this->id) {
			std::string x, y, a;
			ss >> x >> y >> a;
			robots[_id].fieldCoords.x = atoi(x.c_str());
			robots[_id].fieldCoords.y = atoi(y.c_str());
			robots[_id].polarMetricCoords.y = atoi(a.c_str());
		}
	}
}
void Simulator::UpdateGatePos(){

	frame_blank.copyTo(frame);
	blueGate.polarMetricCoords.x = cv::norm(self.fieldCoords - blueGate.fieldCoords);
	blueGate.polarMetricCoords.y = 360 - gDistanceCalculator.angleBetween(cv::Point(0, 1), self.fieldCoords - (blueGate.fieldCoords)) + self.getAngle();
	yellowGate.polarMetricCoords.x = cv::norm(self.fieldCoords - yellowGate.fieldCoords);;
	yellowGate.polarMetricCoords.y = 360 - gDistanceCalculator.angleBetween(cv::Point(0, 1), self.fieldCoords - (yellowGate.fieldCoords)) + self.getAngle();


	for (int s = -1; s < 2; s += 2){
		cv::Point2d shift1(s * 10, -20);
		cv::Point2d shift2(s * 10, 20);
		double a1 = gDistanceCalculator.angleBetween(cv::Point(0, -1), self.fieldCoords - (blueGate.fieldCoords + shift1)) + self.getAngle();
		double a2 = gDistanceCalculator.angleBetween(cv::Point(0, -1), self.fieldCoords - (yellowGate.fieldCoords + shift2)) + self.getAngle();

		double d1 = gDistanceCalculator.getDistanceInverted(self.fieldCoords, blueGate.fieldCoords + shift1);
		double d2 = gDistanceCalculator.getDistanceInverted(self.fieldCoords, yellowGate.fieldCoords + shift2);


		// draw gates
		double x1 = d1*sin(a1 / 180 * CV_PI);
		double y1 = d1*cos(a1 / 180 * CV_PI);
		double x2 = d2*sin(a2 / 180 * CV_PI);
		double y2 = d2*cos(a2 / 180 * CV_PI);

		cv::Scalar color(236, 137, 48);
		cv::Scalar color2(61, 255, 244);
		cv::circle(frame, cv::Point((int)(x1), (int)(y1)) + cv::Point(frame.size() / 2), 28, color, -1);
		cv::circle(frame, cv::Point((int)(x2), (int)(y2)) + cv::Point(frame.size() / 2), 28, color2, -1);
	}

}
void Simulator::UpdateBallPos(double dt){
	// balls 
	for (int i = 0; i < mNumberOfBalls; i++){
		if (isMaster) {
			if (balls[i].speed > 0.001) {
				balls[i].fieldCoords.x += balls[i].speed*dt * (sin(balls[i].heading / 180 * CV_PI));
				balls[i].fieldCoords.y -= balls[i].speed*dt * (cos(balls[i].heading / 180 * CV_PI));
				balls[i].speed *= 0.95;
			}
			std::stringstream message;
			message << "BAL " << i << " " << balls[i].fieldCoords.x << " " << balls[i].fieldCoords.y;
			SendMessage(message.str());
		}
		double a = gDistanceCalculator.angleBetween(cv::Point(0, -1), self.fieldCoords - balls[i].fieldCoords) + self.getAngle();
		double d = gDistanceCalculator.getDistanceInverted(self.fieldCoords, balls[i].fieldCoords);
		double x = d*sin(a / 180 * CV_PI);
		double y = d*cos(a / 180 * CV_PI);
		cv::Scalar color(48, 154, 236);
		cv::circle(frame, cv::Point(x, y) + cv::Point(frame.size() / 2), 12, color, -1);
	}

	// draw shared robots
	for (int i = 0; i < MAX_ROBOTS; i++) {
		if (abs(robots[i].fieldCoords.x) > 1000) continue;
		double a = gDistanceCalculator.angleBetween(cv::Point(0, -1), self.fieldCoords - robots[i].fieldCoords) + self.getAngle();
		double d = gDistanceCalculator.getDistanceInverted(self.fieldCoords, robots[i].fieldCoords);
		double x = d*sin(a / 180 * CV_PI);
		double y = d*cos(a / 180 * CV_PI);
		cv::Scalar color(i * 52, i * 15 + 100, i * 30);
		cv::circle(frame, cv::Point(x, y) + cv::Point(frame.size() / 2), 12, color, -1);
	}

}

void Simulator::UpdateRobotPos(){
	time = boost::posix_time::microsec_clock::local_time();

	double dt = (double)(time - lastStep).total_milliseconds() / 1000.0;
	lastStep = time;

	//if (dt < 0.0000001) return;

	double v = targetSpeed.velocity;
	double w = targetSpeed.rotation;


	self.polarMetricCoords.y += w * dt;
	if (self.polarMetricCoords.y > 360) self.polarMetricCoords.y -= 360;
	if (self.polarMetricCoords.y < -360) self.polarMetricCoords.y += 360;

	self.fieldCoords.x += (int)(v*dt * sin((self.getAngle() + targetSpeed.heading) / 180 * CV_PI));
	self.fieldCoords.y -= (int)(v*dt * cos((self.getAngle() + targetSpeed.heading) / 180 * CV_PI));

	if (isMaster || id > 0) {
		std::stringstream message;
		message << "POS " << id << " " << self.fieldCoords.x << " " << self.fieldCoords.y << " " << self.getAngle();
		SendMessage(message.str());
	}
	UpdateGatePos();
	UpdateBallPos(dt);

	{
		std::lock_guard<std::mutex> lock(mutex);
		frame.copyTo(frame_copy);
	}

}


Simulator::~Simulator()
{
	WaitForStop();
}

cv::Mat & Simulator::Capture(bool bFullFrame){
	if (frames > 10) {
		boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type dt2 = (time - lastCapture2).total_milliseconds();
		fps = 1000.0 * frames / dt2;
		lastCapture2 = time;
		frames = 0;
	}
	else {
		frames++;
	}

	std::lock_guard<std::mutex> lock(mutex);
	frame_copy.copyTo(frame_copy2);
	return frame_copy2;
}

cv::Size Simulator::GetFrameSize(bool bFullFrame){
	return frame.size();
}


double Simulator::GetFPS(){
	return fps;
}


cv::Mat & Simulator::GetLastFrame(bool bFullFrame){
	return frame;
}


void Simulator::TogglePlay(){
}



void Simulator::Drive(double fowardSpeed, double direction, double angularSpeed){
	if (mNumberOfBalls == 0)
		return;
	targetSpeed = { fowardSpeed, direction, angularSpeed };
	/*
	self.polarMetricCoords.y += angularSpeed;
	if (self.polarMetricCoords.y > 360) self.polarMetricCoords.y -= 360;
	if (self.polarMetricCoords.y < -360) self.polarMetricCoords.y += 360;
	self.fieldCoords.x += (int)(fowardSpeed * sin((direction - self.getAngle()) / 180 * CV_PI));
	self.fieldCoords.y += (int)(fowardSpeed * cos((direction - self.getAngle()) / 180 * CV_PI));
	*/
}


const Speed & Simulator::GetActualSpeed(){
	return actualSpeed;
}


const Speed & Simulator::GetTargetSpeed(){
	return targetSpeed;
}


void Simulator::Init(){
}

std::string Simulator::GetDebugInfo() {
	return "simulating wheels";
}

void Simulator::Run(){
	while (!stop_thread){
		UpdateRobotPos();
		Sleep(50);
	}
}

bool Simulator::BallInTribbler(){
	if (!tribblerRunning) return false;
	double minDist = INT_MAX;
	double dist = INT_MAX;
	for (int i = 0; i < mNumberOfBalls; i++){
		dist = cv::norm(self.fieldCoords - balls[i].fieldCoords);
		//std::cout << dist << std::endl;
		if (dist < minDist)
			minDist = dist;
	}
	if (minDist < 15)
		return true;
	else return false;
}

void Simulator::Kick(){
	double minDist = INT_MAX;
	double dist = INT_MAX;
	int minDistIndex = mNumberOfBalls - 1;
	for (int i = 0; i < mNumberOfBalls; i++){
		dist = cv::norm(self.fieldCoords - balls[i].fieldCoords);
		if (dist < minDist){
			minDist = dist;
			minDistIndex = i;
		}
	}
	if (isMaster) {
		balls[minDistIndex].speed = 600;
		balls[minDistIndex].heading = self.getAngle();
	}
	else {
		SendMessage("KCK" + std::to_string(id) + " 600 " + std::to_string(self.getAngle()));
	}
	//balls[minDistIndex] = balls[mNumberOfBalls - 1];
	//balls[mNumberOfBalls - 1].~BallPosition();
	//mNumberOfBalls--;
}