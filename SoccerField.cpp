#include "SoccerField.h"
#include <chrono>
#include <thread>
#include <boost/system/error_code.hpp>
#include "DistanceCalculator.h"

extern DistanceCalculator gDistanceCalculator;

SoccerField::SoccerField(boost::asio::io_service &io, IDisplay *pDisplay, bool master, int number_of_balls, int port) :m_pDisplay(pDisplay)
, UdpServer(io, port, master), FieldState(number_of_balls), ThreadedClass("SoccerField")
{
	green = cv::imread("field.png", CV_LOAD_IMAGE_COLOR);   // Read the file
	field = cv::Mat(green.size(), CV_8UC3, cv::Scalar::all(245));
	c = cv::Point2d(green.size()) / 2;

	Start();
}


SoccerField::~SoccerField()
{
	WaitForStop();
	boost::system::error_code error;

}
void SoccerField::SetTargetGate(OBJECT gate) {
	m_targetGate = gate;
	partner.fieldCoords = GetHomeGate().fieldCoords + cv::Point2d(0, 100);
	partner.polarMetricCoords.y = gDistanceCalculator.angleBetween(partner.fieldCoords, { 0, -1 });
	partner.polarMetricCoords.x = cv::norm(self.fieldCoords - partner.fieldCoords);
};
GatePosition & SoccerField::GetTargetGate() {
	if (m_targetGate == BLUE_GATE) return blueGate;
	else if (m_targetGate == YELLOW_GATE) return yellowGate;
	else return blueGate; // { return{ -1, 0 }; }
};

GatePosition & SoccerField::GetHomeGate() {
	if (m_targetGate == BLUE_GATE) return yellowGate;
	else if (m_targetGate == YELLOW_GATE) return blueGate;
	else return yellowGate; // { return{ -1, 0 }; }
};

void SoccerField::initBalls() {

}

void SoccerField::Run(){
	while (!stop_thread){

		//recvState();
		green.copyTo(field);

		{
			std::stringstream message;
			message << "POS " << " " << self.fieldCoords.x << " " << self.fieldCoords.y << " " << self.getAngle() << " #";
			SendMessage(message.str());
		}

		cv::circle(field, self.rawFieldCoords + c, 24, cv::Scalar(0, 33, 255), 4);
		cv::circle(field, self.fieldCoords + c, 14, cv::Scalar(133, 33, 55), 4);
		cv::line(field, self.fieldCoords + c,
			cv::Point2d((40.0*sin(self.getAngle() / 360 * TAU)),( -40 * cos(self.getAngle() / 360 * TAU)))
			+ self.fieldCoords + c
			, cv::Scalar(133, 33, 55), 3);
		
		for (int i = 0; i < balls.size(); i++) {
			BallPosition &_ball = balls[i];
			cv::circle(field, _ball.fieldCoords + c, 7, cv::Scalar(48, 154, 236), -1);
			/*{
				message << "BAL " <<(int)balls[i].fieldCoords.x << " " << (int)balls[i].fieldCoords.y << " ";
				//SendMessage(message.str());
			}*/
		}
		
		if (blueGate.getDistance() > 0) {
			cv::circle(field, blueGate.fieldCoords + c, 14, cv::Scalar(236, 137, 48), 7);
			cv::circle(field, blueGate.fieldCoords + c, (int)(blueGate.polarMetricCoords.x), cv::Scalar(236, 137, 48), 2);

			cv::line(field, self.fieldCoords + c,
				cv::Point2d((blueGate.polarMetricCoords.x*sin((blueGate.polarMetricCoords.y+self.getAngle()) / 180 * CV_PI)),
				(-blueGate.polarMetricCoords.x*cos((blueGate.polarMetricCoords.y+self.getAngle()) / 180 * CV_PI))
				) + self.fieldCoords + c
				, cv::Scalar(236, 137, 48), 3);
		}

		if (yellowGate.getDistance() > 0) {
			cv::circle(field, yellowGate.fieldCoords + c, 14, cv::Scalar(61, 255, 244), 7);
			cv::circle(field, yellowGate.fieldCoords + c, (int)(yellowGate.polarMetricCoords.x), cv::Scalar(61, 255, 244), 2);

			cv::line(field, self.fieldCoords + c,
				cv::Point2d((yellowGate.polarMetricCoords.x*sin((yellowGate.polarMetricCoords.y + self.getAngle()) / 360 * TAU)),
				(-yellowGate.polarMetricCoords.x*cos((yellowGate.polarMetricCoords.y + self.getAngle()) / 360 * TAU))
				) + self.fieldCoords + c
				, cv::Scalar(61, 255, 244), 3);
		}

		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(150));

	}
}

void SoccerField::MessageReceived(const std::string & message){
	std::stringstream ss(message);
	std::string command;
	ss >> command;

	if (command == "KCK") {
		;
	}
	else if (command == "BAL") {
		;
	}
	else  if (command == "PAS") {
		gameMode = GAME_MODE_TAKE_BALL;
	}
	else  if (command == "POS") {
		std::string x, y, a;
		ss >> x >> y >> a;
		partner.fieldCoords.x = atoi(x.c_str());
		partner.fieldCoords.y = atoi(y.c_str());
		partner.polarMetricCoords.y = gDistanceCalculator.angleBetween(partner.fieldCoords, { 0, -1 });
		partner.polarMetricCoords.x = cv::norm(self.fieldCoords - partner.fieldCoords);
	}
}