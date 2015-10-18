#include "Simulator.h"
#include "DistanceCalculator.h"
#include <chrono>
#include <thread>
extern DistanceCalculator gDistanceCalculator;

Simulator::Simulator() :ThreadedClass("Simulator")
{
	self.fieldCoords = cv::Point(50, 50);
	self.polarMetricCoords = cv::Point(0,0);
	// distribute balls uniformly
	for (int i = 0; i < NUMBER_OF_BALLS; i++) {
		balls[i].fieldCoords.x = (int)(((i % 3) - 1) * 100);
		balls[i].fieldCoords.y = (int)((i / 3 - 1.5) * 110);
		balls[i].id = i;
	}
	Start();
}
void Simulator::UpdateGatePos(){

	frame_blank.copyTo(frame);
	blueGate.polarMetricCoords.x = cv::norm(self.fieldCoords- blueGate.fieldCoords);
	blueGate.polarMetricCoords.y = 360-gDistanceCalculator.angleBetween(cv::Point(0, 1), self.fieldCoords - (blueGate.fieldCoords)) + self.getAngle();
	yellowGate.polarMetricCoords.x = cv::norm(self.fieldCoords - yellowGate.fieldCoords);;
	yellowGate.polarMetricCoords.y = 360 - gDistanceCalculator.angleBetween(cv::Point(0, 1), self.fieldCoords - (yellowGate.fieldCoords)) + self.getAngle();


	for (int s = -1; s < 2; s += 2){
		cv::Point2i shift1(s * 10, -20);
		cv::Point2i shift2(s * 10, 20);
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
		cv::circle(frame, cv::Point((int)(x2), (int)(y2))+cv::Point(frame.size() / 2), 28, color2, -1);
	}


	// balls 
	for (int i = 0; i < mNumberOfBalls; i++){
		double a = gDistanceCalculator.angleBetween(cv::Point(0, -1), self.fieldCoords - balls[i].fieldCoords) + self.getAngle();
		double d = gDistanceCalculator.getDistanceInverted(self.fieldCoords, balls[i].fieldCoords);
		double x = d*sin(a / 180 * CV_PI);
		double y = d*cos(a / 180 * CV_PI);
		cv::Scalar color(48, 154, 236);
		cv::circle(frame, cv::Point(x, y) + cv::Point(frame.size() / 2), 12, color, -1);
	}

	{
		std::lock_guard<std::mutex> lock(mutex);
		frame.copyTo(frame_copy);
	}

}

void Simulator::UpdateRobotPos(){
	UpdateGatePos();
}


Simulator::~Simulator()
{
	WaitForStop();
}

cv::Mat & Simulator::Capture(bool bFullFrame){

	std::lock_guard<std::mutex> lock(mutex);
	frame_copy.copyTo(frame_copy2);
	return frame_copy2;
}

cv::Size Simulator::GetFrameSize(bool bFullFrame){
	return frame.size();
}


double Simulator::GetFPS(){
	return 0;
}


cv::Mat & Simulator::GetLastFrame(bool bFullFrame){
	return frame;
}


void Simulator::TogglePlay(){
}



void Simulator::Drive(double fowardSpeed, double direction, double angularSpeed){
	if (mNumberOfBalls == 0)
		return;
	self.polarMetricCoords.y += angularSpeed;
	if (self.polarMetricCoords.y > 360) self.polarMetricCoords.y -= 360;
	if (self.polarMetricCoords.y < -360) self.polarMetricCoords.y += 360;
	self.fieldCoords.x += (int)(fowardSpeed * sin((direction - self.getAngle()) / 180 * CV_PI));
	self.fieldCoords.y += (int)(fowardSpeed * cos((direction - self.getAngle()) / 180 * CV_PI));
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
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

bool Simulator::BallInTribbler(){
	double minDist = INT_MAX;
	double dist = INT_MAX;
	for (int i = 0; i < mNumberOfBalls; i++){
		dist = cv::norm(self.fieldCoords - balls[i].fieldCoords);
		//std::cout << dist << std::endl;
		if (dist < minDist)
			minDist = dist;
	}
	if (minDist < 40)
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
	balls[minDistIndex] = balls[mNumberOfBalls - 1];
	balls[mNumberOfBalls - 1].~BallPosition();
	mNumberOfBalls--;
}