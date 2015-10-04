#pragma once
#include "types.h"
#include "FieldState.h"
#include "ThreadedClass.h"

class Simulator: public ICamera, public IWheelController, public ThreadedClass
{
public:
	Simulator();
	virtual ~Simulator();

	virtual cv::Mat & Capture(bool bFullFrame = false);
	virtual cv::Size GetFrameSize(bool bFullFrame = false);
	virtual double GetFPS();
	virtual cv::Mat & GetLastFrame(bool bFullFrame = false);
	virtual void TogglePlay();

	virtual void Drive(double fowardSpeed, double direction = 0, double angularSpeed = 0);
	virtual const Speed & GetActualSpeed();
	virtual const Speed & GetTargetSpeed();
	virtual void Init();
	std::string GetDebugInfo();
	bool IsReal(){ return false;  }
	void Run();

protected:
	double orientation;
	cv::Mat frame = cv::Mat(1024, 1280, CV_8UC3);
	cv::Mat frame_blank = cv::Mat(1024, 1280, CV_8UC3);
	Speed targetSpeed, actualSpeed;
	FieldState fieldState;
	void UpdateGatePos();
	void UpdateRobotPos();

};

