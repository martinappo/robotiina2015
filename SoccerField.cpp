#include "SoccerField.h"
#include <chrono>
#include <thread>

SoccerField::SoccerField(IDisplay *pDisplay, cv::Size frameSize) :m_pDisplay(pDisplay)
{
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
	ObjectPosition _blueGate;
	ObjectPosition _yellowGate;

	while (!stop_thread){
		_blueGate = blueGate.load();
		_yellowGate = yellowGate.load();
		green.copyTo(field);
		cv::circle(field, cv::Point(320, 240), 14, cv::Scalar(133, 33, 55), 4);

		for (int i = 0; i < NUMBER_OF_BALLS; i++) {
			BallPosition ball = balls[i].load();
			cv::circle(field, ball.pixelCoordsForField, 7, cv::Scalar(48, 154, 236), -1);
			ball.setIsUpdated(false);
			balls[i].store(ball);
		}

		if (_blueGate.getDistance() > 0) {
			cv::circle(field, _blueGate.pixelCoordsForField, 14, cv::Scalar(236, 137, 48), 7);
		}
		if (_yellowGate.getDistance() > 0) {
			cv::circle(field, _yellowGate.pixelCoordsForField, 14, cv::Scalar(61, 255, 244), 7);
		}
		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}