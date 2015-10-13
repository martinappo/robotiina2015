#pragma once
#include "types.h"
#include "FieldState.h"
#include "ThreadedClass.h"
#include <mutex>
class Simulator: public ICamera, public IWheelController, public ICoilGun, public ThreadedClass, public FieldState
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
	virtual void SetTargetGate(OBJECT gate) {}
	virtual GatePosition &GetTargetGate() { return blueGate; };
	virtual bool BallInTribbler() {
		// check if ball is infront of robot
		throw std::runtime_error("Implement me");
	};
	virtual void Kick() {
		//remove ball from field
		throw std::runtime_error("Implement me");
	};
	virtual void ToggleTribbler(bool start) {
		// do nothing
	};
protected:
	double orientation;
	cv::Mat frame = cv::Mat(1024, 1280, CV_8UC3);
	cv::Mat frame_copy = cv::Mat(1024, 1280, CV_8UC3);
	cv::Mat frame_copy2 = cv::Mat(1024, 1280, CV_8UC3);
	cv::Mat frame_blank = cv::Mat(1024, 1280, CV_8UC3, cv::Scalar(21, 188, 80));
	Speed targetSpeed, actualSpeed;
	void UpdateGatePos();
	void UpdateRobotPos();
	std::mutex mutex;
};

