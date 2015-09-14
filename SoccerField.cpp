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


		cv::circle(field, self.fieldCoords + cv::Point(250, 150), 14, cv::Scalar(133, 33, 55), 4);

		for (int i = 0; i < NUMBER_OF_BALLS; i++) {
			BallPosition &_ball = balls[i];
			cv::circle(field, _ball.fieldCoords + cv::Point(250, 150), 7, cv::Scalar(48, 154, 236), -1);
			_ball.setIsUpdated(false);
		}

		if (blueGate.getDistance() > 0) {
			cv::circle(field, blueGate.fieldCoords + cv::Point(250,150), 14, cv::Scalar(236, 137, 48), 7);
			cv::circle(field, blueGate.fieldCoords + cv::Point(250, 150), blueGate.polarMetricCoords.x, cv::Scalar(236, 137, 48), 2);
			cv::line(field, blueGate.fieldCoords + cv::Point(250, 150),
				cv::Point((double)blueGate.polarMetricCoords.x*cos(blueGate.polarMetricCoords.y/360*TAU),
				(double)blueGate.polarMetricCoords.x*sin(blueGate.polarMetricCoords.y / 360 * TAU)
					) + cv::Point(250, 150)
				, cv::Scalar(236, 137, 48));

		}
		std::cout << blueGate.polarMetricCoords.x << ", " << blueGate.polarMetricCoords.y << ", " << (double)blueGate.polarMetricCoords.x*cos(blueGate.polarMetricCoords.y / 360 * TAU) << " <==> ";
		std::cout << yellowGate.polarMetricCoords.x << ", " << yellowGate.polarMetricCoords.y << ", " << (double)yellowGate.polarMetricCoords.x*cos(yellowGate.polarMetricCoords.y / 360 * TAU) << std::endl;

		if (yellowGate.getDistance() > 0) {
			cv::circle(field, yellowGate.fieldCoords + cv::Point(250, 150), 14, cv::Scalar(61, 255, 244), 7);
			cv::circle(field, yellowGate.fieldCoords + cv::Point(250, 150), yellowGate.polarMetricCoords.x, cv::Scalar(61, 255, 244), 2);
			
			cv::line(field, yellowGate.fieldCoords + cv::Point(250, 150),
				cv::Point(yellowGate.polarMetricCoords.x*cos(yellowGate.polarMetricCoords.y / 360 * TAU),
				yellowGate.polarMetricCoords.x*sin(yellowGate.polarMetricCoords.y / 360 * TAU)
				) + cv::Point(250, 150)
				, cv::Scalar(61, 255, 244), 3);
			
		}
		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}