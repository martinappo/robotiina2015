#include "Simulator.h"


Simulator::Simulator()
{
}


Simulator::~Simulator()
{
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


