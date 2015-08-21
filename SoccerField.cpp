#include "SoccerField.h"
#include <chrono>
#include <thread>

SoccerField::SoccerField(IDisplay *pDisplay, cv::Size frameSize) :m_pDisplay(pDisplay)
{
	this->self = RobotPosition(this->yellowGate, this->blueGate, cv::Point(214, 180));
	initBalls(frameSize);
	Start();
}


SoccerField::~SoccerField()
{
	WaitForStop();
}
ObjectPosition SoccerField::GetTargetGate() const {
	if (m_targetGate == BLUE_GATE) return blueGate;
	else if (m_targetGate == YELLOW_GATE) return yellowGate;
	else { return { -1, 0 }; }
};

void SoccerField::initBalls(cv::Size frameSize) {
	for (int i = 0; i < NUMBER_OF_BALLS; i++) {
		BallPosition ball = balls[i].load();
		ball.frameSize = frameSize;
		balls[i].store(ball);
	}
}

void SoccerField::Run(){
	GatePosition _blueGate;
	GatePosition _yellowGate;
	RobotPosition _robot;

	while (!stop_thread){
		_blueGate = blueGate.load();
		_yellowGate = yellowGate.load();
		_robot = self.load();
		green.copyTo(field);


		cv::circle(field, _robot.fieldCoords, 14, cv::Scalar(133, 33, 55), 4);

		for (int i = 0; i < NUMBER_OF_BALLS; i++) {
			BallPosition _ball = balls[i].load();
			cv::circle(field, _ball.fieldCoords, 7, cv::Scalar(48, 154, 236), -1);
			_ball.setIsUpdated(false);
			balls[i].store(_ball);
		}

		if (_blueGate.getDistance() > 0) {
			cv::circle(field, _blueGate.fieldCoords, 14, cv::Scalar(236, 137, 48), 7);
		}
		if (_yellowGate.getDistance() > 0) {
			cv::circle(field, _yellowGate.fieldCoords, 14, cv::Scalar(61, 255, 244), 7);
		}
		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}