#include "SoccerField.h"
#include <chrono>
#include <thread>
#include <boost/system/error_code.hpp>
SoccerField::SoccerField(boost::asio::io_service &io, IDisplay *pDisplay, bool master, int port) :m_pDisplay(pDisplay)
, UdpServer(io, port, master)
{
	green = cv::imread("field.png", CV_LOAD_IMAGE_COLOR);   // Read the file
	field = cv::Mat(green.size(), CV_8UC3, cv::Scalar::all(245));
	c = cv::Point2d(green.size()) / 2;

	initBalls();
	Start();
}


SoccerField::~SoccerField()
{
	WaitForStop();
	boost::system::error_code error;

}

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
	// distribute balls uniformly
	for (int i = 0; i < NUMBER_OF_BALLS; i++) {
		balls[i].fieldCoords.x = (int)(((i % 3) - 1) * 100);
		balls[i].fieldCoords.y = (int)((i / 3 - 1.5) * 110);
		balls[i].id = i;
	}
}
void SoccerField::sendState(){
	std::string message = "testing 123";
	//SendMessage(message);

}

void SoccerField::recvState(){
	/*
	// Receive data.
	boost::array<char, 4> buffer;
	std::size_t bytes_transferred =
		socket.receive_from(boost::asio::buffer(buffer), senderEndpoint);

	std::cout << "got " << bytes_transferred << " bytes." << std::endl;
	*/
}



void SoccerField::Run(){
	while (!stop_thread){

		sendState();
		//recvState();
		green.copyTo(field);


		cv::circle(field, self.rawFieldCoords + c, 24, cv::Scalar(0, 33, 255), 4);
		cv::circle(field, self.fieldCoords + c, 14, cv::Scalar(133, 33, 55), 4);
		cv::line(field, self.fieldCoords + c,
			cv::Point2d((40.0*sin(self.getAngle() / 360 * TAU)),( -40 * cos(self.getAngle() / 360 * TAU)))
			+ self.fieldCoords + c
			, cv::Scalar(133, 33, 55), 3);


		
		for (int i = 0; i < NUMBER_OF_BALLS; i++) {
			BallPosition &_ball = balls[i];
			cv::circle(field, _ball.fieldCoords + c, 7, cv::Scalar(48, 154, 236), -1);
			//_ball.setIsUpdated(false);
		}
		
		if (blueGate.getDistance() > 0) {
			cv::circle(field, blueGate.fieldCoords + c, 14, cv::Scalar(236, 137, 48), 7);
			cv::circle(field, blueGate.fieldCoords + c, (int)(blueGate.polarMetricCoords.x), cv::Scalar(236, 137, 48), 2);
			/*
			cv::line(field, blueGate.fieldCoords + c,
				cv::Point(blueGate.polarMetricCoords.x*sin(blueGate.polarMetricCoords.y/360*TAU),
										blueGate.polarMetricCoords.x*cos(blueGate.polarMetricCoords.y / 360 * TAU)
				) + blueGate.fieldCoords + c
				, cv::Scalar(236, 137, 48),3);
				*/
			cv::line(field, self.fieldCoords + c,
				cv::Point2d((blueGate.polarMetricCoords.x*sin((blueGate.polarMetricCoords.y+self.getAngle()) / 180 * CV_PI)),
				(-blueGate.polarMetricCoords.x*cos((blueGate.polarMetricCoords.y+self.getAngle()) / 180 * CV_PI))
				) + self.fieldCoords + c
				, cv::Scalar(236, 137, 48), 3);
		}
		/*
		std::cout << blueGate.polarMetricCoords.x << ", " << blueGate.polarMetricCoords.y << ", " 
			<< blueGate.polarMetricCoords.x*cos(blueGate.polarMetricCoords.y / 360 * TAU) << ", "
			<< blueGate.polarMetricCoords.x*sin(blueGate.polarMetricCoords.y / 360 * TAU) << " <==> ";
		std::cout << yellowGate.polarMetricCoords.x << ", " << yellowGate.polarMetricCoords.y << ", " 
			<< yellowGate.polarMetricCoords.x*cos(yellowGate.polarMetricCoords.y / 360 * TAU) << ", "
			<< yellowGate.polarMetricCoords.x*sin(yellowGate.polarMetricCoords.y / 360 * TAU) << std::endl;
		*/
		if (yellowGate.getDistance() > 0) {
			cv::circle(field, yellowGate.fieldCoords + c, 14, cv::Scalar(61, 255, 244), 7);
			cv::circle(field, yellowGate.fieldCoords + c, (int)(yellowGate.polarMetricCoords.x), cv::Scalar(61, 255, 244), 2);
			/*
			cv::line(field, yellowGate.fieldCoords + c,
				 cv::Point(-yellowGate.polarMetricCoords.x*sin(yellowGate.polarMetricCoords.y / 360 * TAU),
				yellowGate.polarMetricCoords.x*cos(yellowGate.polarMetricCoords.y / 360 * TAU)
				) + yellowGate.fieldCoords + c
				, cv::Scalar(61, 255, 244), 3);
			*/
			cv::line(field, self.fieldCoords + c,
				cv::Point2d((yellowGate.polarMetricCoords.x*sin((yellowGate.polarMetricCoords.y + self.getAngle()) / 360 * TAU)),
				(-yellowGate.polarMetricCoords.x*cos((yellowGate.polarMetricCoords.y + self.getAngle()) / 360 * TAU))
				) + self.fieldCoords + c
				, cv::Scalar(61, 255, 244), 3);
		}
		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}