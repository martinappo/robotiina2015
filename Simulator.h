#pragma once
#include "types.h"
#include "FieldState.h"
#include "ThreadedClass.h"
#include "UdpServer.h"
#include "refereeCom.h"

#include <mutex>

const int MAX_ROBOTS = 10;
class Simulator : public ICamera, public ISerial, public ThreadedClass, public FieldState, public UdpServer, public RefereeCom
{
  using UdpServer::SendMessage;
public:
	Simulator(boost::asio::io_service &io, bool master, const std::string game_mode);
	virtual ~Simulator();

	virtual cv::Mat & Capture(bool bFullFrame = false);
	virtual cv::Size GetFrameSize(bool bFullFrame = false);
	virtual double GetFPS();
	virtual cv::Mat & GetLastFrame(bool bFullFrame = false);
	virtual void TogglePlay();
	void CalcRobotSpeed(double dt);
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

	virtual void DataReceived(const std::string & message){};//serial
	virtual void MessageReceived(const std::string & message); // UDP

	void giveCommand(FieldState::GameMode command);
	typedef	void(*MessageCallback)(const std::string & message);
	virtual void WriteString(const std::string &s);
	//virtual void DataReceived(const std::string & message){};
	virtual void SetMessageHandler(MessageCallback *callback){
		messageCallback = callback;
	};

protected:
	MessageCallback *messageCallback = NULL;

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
	cv::Mat wheelSpeeds = cv::Scalar::all(0.0);
};

