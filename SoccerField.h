#pragma once
#include "ThreadedClass.h"
#include "types.h"
#include <boost/thread/mutex.hpp>
#include "FieldState.h"
#include "ObjectPosition.h"
#include "UdpServer.h"
const int MAX_ROBOTS_NR = 10;

class SoccerField :
	public UdpServer, ThreadedClass, public FieldState
{
public:
	SoccerField(boost::asio::io_service &io, IDisplay *pDisplay, bool master, int number_of_balls, int port=45000);
	virtual ~SoccerField();
	void Run();
	virtual void SetTargetGate(OBJECT gate) {
		m_targetGate = gate;
	};
	virtual GatePosition &GetTargetGate();
	virtual GatePosition &GetHomeGate();
	void initBalls();
	void Lock(){};
	void UnLock(){};
	virtual void MessageReceived(const std::string & message);

private:
	std::atomic_int m_targetGate;
	IDisplay *m_pDisplay;
	//310cm x 460+40cm <-- field dimensions. These values suit perfectly for pixel values also :)
	//+ 40 cm for gates
	//const cv::Mat green = cv::Mat(310, 500, CV_8UC3, cv::Scalar(21, 188, 80));
	cv::Mat green;
	cv::Mat field;// = cv::Mat(310, 500, CV_8UC3, cv::Scalar::all(245)); // blink display
	cv::Point2d c/*enter*/;
	void sendState();
	void recvState();
	
	bool isMaster = false;
	bool isMasterPresent = false;
	int id = 0;
	int next_id = 1;
	bool stop_send = false;


};

