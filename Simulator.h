#pragma once
#include "types.h"
#include "FieldState.h"
#include "ThreadedClass.h"
#include "UdpServer.h"
#include "refereeCom.h"

#include <mutex>

const int MAX_ROBOTS = 10;
class Simulator : public ICamera/*, public IRefereeCom*/, public ICommunicationModule, public ThreadedClass, public FieldState, public UdpServer, public RefereeCom
{
public:
	Simulator(boost::asio::io_service &io, bool master, const std::string game_mode);
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
	virtual GatePosition &GetHomeGate() { return yellowGate; };
	virtual bool BallInTribbler();
	virtual void Kick(int force = 800);
	virtual void ToggleTribbler(bool start) {
		tribblerRunning = start;
	};

	virtual void MessageReceived(const std::string & message);

	void giveCommand(FieldState::GameMode command);

protected:
	double orientation;
	cv::Mat frame = cv::Mat(1024, 1280, CV_8UC3);
	cv::Mat frame_copy = cv::Mat(1024, 1280, CV_8UC3);
	cv::Mat frame_copy2 = cv::Mat(1024, 1280, CV_8UC3);
	cv::Mat frame_blank = cv::Mat(1024, 1280, CV_8UC3, cv::Scalar(21, 188, 80));
	Speed targetSpeed, actualSpeed;
	void UpdateGatePos();
	void UpdateBallPos(double dt);
	void UpdateRobotPos();
	std::mutex mutex;
	ObjectPosition robots[MAX_ROBOTS];

private:
	int mNumberOfBalls;
	int frames = 0;
	double fps;
	bool tribblerRunning = false;
	boost::posix_time::ptime lastCapture2;
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
	bool isMaster = false;
	bool isMasterPresent = false;
	int id = -1;
	int next_id = 1;
	bool stop_send = false;
	bool ball_in_tribbler = false;

};

