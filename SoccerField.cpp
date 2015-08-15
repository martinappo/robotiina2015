#include "SoccerField.h"
#include <chrono>
#include <thread>

SoccerField::SoccerField(IDisplay *pDisplay) :m_pDisplay(pDisplay)
{
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

cv::Point2i SoccerField::Polar2Cartesian(ObjectPosition pos) const {
	float y = pos.getDistance() * cos(TAU*pos.getAngle() / 360) / 16;
	float x = pos.getDistance() * sin(TAU*pos.getAngle() / 360) / 16;
	return cv::Point(320 + x, 240 - y);
}

void SoccerField::Run(){
	ObjectPosition _blueGate;
	ObjectPosition _yellowGate;
	ObjectPosition _ball;
	while (!stop_thread){
		_ball = balls[0].load();
		_blueGate = blueGate.load();
		_yellowGate = yellowGate.load();
		green.copyTo(field);
		cv::circle(field, cv::Point(320, 240), 14, cv::Scalar(133, 33, 55), 4);

		cv::Point2i filteredBallPos = cv::Point(-1, -1);
		if (_ball.getDistance() < 0) {
			filteredBallPos = filter->doFiltering(filteredBallPos);
		}
		else {
			filteredBallPos = filter->doFiltering(Polar2Cartesian(_ball));
		}
		 
		cv::circle(field, filteredBallPos, 7, cv::Scalar(48, 154, 236), 4);

		if (_blueGate.getDistance() > 0) {
			cv::circle(field, Polar2Cartesian(_blueGate), 14, cv::Scalar(236, 137, 48), 7);
		}
		if (_yellowGate.getDistance() > 0) {
			cv::circle(field, Polar2Cartesian(_yellowGate), 14, cv::Scalar(61, 255, 244), 7);
		}
		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}