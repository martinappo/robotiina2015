#include "Simulator.h"
#include "DistanceCalculator.h"
#include <chrono>
#include <thread>
extern DistanceCalculator gDistanceCalculator;

Simulator::Simulator() :ThreadedClass("Simulator")
{
	for (int i = 0; i < NUMBER_OF_BALLS; i++){
		fieldState.balls[i].fieldCoords = cv::Point(i / 4, i % 4) - cv::Point(303,303);
	}
	fieldState.self.fieldCoords = cv::Point(0, 0);
	fieldState.self.polarMetricCoords = cv::Point(0,0);
	Start();
}
void Simulator::UpdateGatePos(){
	//double a1 = gDistanceCalculator.angleBetween()
}

void Simulator::UpdateRobotPos(){
}


Simulator::~Simulator()
{
	WaitForStop();
}

cv::Mat & Simulator::Capture(bool bFullFrame){
	return frame;
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