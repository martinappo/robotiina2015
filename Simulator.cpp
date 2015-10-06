#include "Simulator.h"
#include "DistanceCalculator.h"
#include <chrono>
#include <thread>
extern DistanceCalculator gDistanceCalculator;

Simulator::Simulator() :ThreadedClass("Simulator")
{
	self.fieldCoords = cv::Point(-100, 110);
	self.polarMetricCoords = cv::Point(0,0);
	// distribute balls uniformly
	for (int i = 0; i < NUMBER_OF_BALLS; i++) {
		balls[i].fieldCoords.x = ((i % 3) - 1) * 100;
		balls[i].fieldCoords.y = (i / 3 - 1.5) * 110;
		balls[i].id = i;
	}
	Start();
}
void Simulator::UpdateGatePos(){

	frame_blank.copyTo(frame);
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
		cv::circle(frame, cv::Point(x1, y1) + cv::Point(frame.size() / 2), 28, color, -1);
		cv::circle(frame, cv::Point(x2, y2) + cv::Point(frame.size() / 2), 28, color2, -1);
	}
	// balls 
	for (int i = 0; i < NUMBER_OF_BALLS; i++){
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
	self.polarMetricCoords.y += angularSpeed;
	self.fieldCoords.x += fowardSpeed * sin((direction - self.getAngle()) / 180 * CV_PI);
	self.fieldCoords.y += fowardSpeed * cos((direction - self.getAngle()) / 180 * CV_PI);
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