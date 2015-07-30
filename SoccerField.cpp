#include "SoccerField.h"
#include <chrono>
#include <thread>

SoccerField::SoccerField(IDisplay *pDisplay) :m_pDisplay(pDisplay)
{
	Start();
}


SoccerField::~SoccerField()
{
}

void SoccerField::Run(){
	ObjectPosition _blueGate;
	ObjectPosition _yellowGate;
	ObjectPosition _ball;
	while (!stop_thread){
		_ball = balls[0];
		green.copyTo(field);
		cv::circle(field, cv::Point(320, 240), 14, cv::Scalar(133, 33, 55), 4);
		if (_ball.distance > 0) {
			float x = _ball.distance * cos(TAU/_ball.horizontalAngle) / 8;
			float y = _ball.distance * sin(TAU/_ball.horizontalAngle) / 8;
			cv::circle(field, cv::Point(320 + x, 240 + y), 7, cv::Scalar(48, 154, 236), 4);

		}
		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}