#include "SoccerField.h"
#include <chrono>
#include <thread>

SoccerField::SoccerField(IDisplay *pDisplay, cv::Size frameSize) :m_pDisplay(pDisplay)
{
	this->self.setFrameSize(frameSize);
	initBalls(frameSize);
	Start();
}


SoccerField::~SoccerField()
{
	WaitForStop();
}

GatePosition & SoccerField::GetTargetGate() {
	if (m_targetGate == BLUE_GATE) return blueGate;
	else if (m_targetGate == YELLOW_GATE) return yellowGate;
	else return blueGate; // { return{ -1, 0 }; }
};

void SoccerField::initBalls(cv::Size frameSize) {
	for (int i = 0; i < NUMBER_OF_BALLS; i++) {
		balls[i].setFrameSize(frameSize);
	}
}

void SoccerField::Run(){
	while (!stop_thread){
		green.copyTo(field);


		cv::circle(field, self.fieldCoords, 14, cv::Scalar(133, 33, 55), 4);

		for (int i = 0; i < NUMBER_OF_BALLS; i++) {
			BallPosition &_ball = balls[i];
			cv::circle(field, _ball.fieldCoords, 7, cv::Scalar(48, 154, 236), -1);
			_ball.setIsUpdated(false);
		}

		if (blueGate.getDistance() > 0) {
			cv::circle(field, blueGate.fieldCoords, 14, cv::Scalar(236, 137, 48), 7);
		}
		if (yellowGate.getDistance() > 0) {
			cv::circle(field, yellowGate.fieldCoords, 14, cv::Scalar(61, 255, 244), 7);
		}
		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}