#include "SoccerField.h"
#include <chrono>
#include <thread>
#include <boost/system/error_code.hpp>

SoccerField::SoccerField(boost::asio::io_service &io, IDisplay *pDisplay, bool master, int number_of_balls, int port) :m_pDisplay(pDisplay)
, UdpServer(io, port, master), FieldState(number_of_balls)
{
	green = cv::imread("field.png", CV_LOAD_IMAGE_COLOR);   // Read the file
	field = cv::Mat(green.size(), CV_8UC3, cv::Scalar::all(245));
	c = cv::Point2d(green.size()) / 2;
	
	isMaster = master;
	if (isMaster)
		initBalls();
	else SendMessage("ID? #"); // ask slave id
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

		std::stringstream message;
		message << "STT " << balls.size() << " ";

		sendState();
		//recvState();
		green.copyTo(field);

		if (!isMaster && id > 0) {
			std::stringstream message;
			message << "POS " << id << " " << self.fieldCoords.x << " " << self.fieldCoords.y << " " << self.getAngle() << " #";
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
			if (isMaster) {
				message << (int)balls[i].fieldCoords.x << " " << (int)balls[i].fieldCoords.y << " ";
				//SendMessage(message.str());
			}
		}
		if (isMaster) {
			message << 0 << " " << self.fieldCoords.x << " " << self.fieldCoords.y << " ";
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

		for (int i = 0; i < MAX_ROBOTS_NR; i++) {
			if (abs(robots[i].fieldCoords.x) > 1000) continue;
			if (isMaster) {
				message << i << " " << (int)robots[i].fieldCoords.x << " " << (int)robots[i].fieldCoords.y << " ";
			}
		}
		if (isMaster) {
			message << " 99 0 0 #";
			SendMessage(message.str());
		}

		m_pDisplay->ShowImage(field, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}

void SoccerField::MessageReceived(const std::string & message){
	std::stringstream ss(message);
	std::string command, r_id;
	ss >> command;
	if (isMaster) {
		if (command == "ID?") {
			stop_send = true;
			SendMessage("ID= " + std::to_string(next_id++) + " #");
		}
		else if (command == "POS") { // foward to slaves
			SendMessage(message);
		}
		else if (command == "ACK") { // id received
			stop_send = false;
		}
		else if (command == "KCK") {
			double s, a;
			ss >> r_id >> s >> a;
			int _id = atoi(r_id.c_str());
			balls[_id].speed = s;
			balls[_id].heading = a;
		}
	}
	else { // slave commands
		if (command == "ID=") {
			ss >> r_id;
			id = atoi(r_id.c_str());
			SendMessage("ACK #");
		}
		else if (command == "BAL") {
			ss >> r_id;
			int _id = atoi(r_id.c_str());
			if (_id != id) {
				std::string x, y, a;
				ss >> x >> y;
				balls[_id].fieldCoords.x = atof(x.c_str());
				balls[_id].fieldCoords.y = atof(y.c_str());
			}
		}
		else if (command == "STT") {
			int numballs;
			ss >> numballs;
			for (int i = 0; i < balls.size(); i++) {
				std::string x, y, a;
				ss >> x >> y;
				balls[i].fieldCoords.x = atof(x.c_str());
				balls[i].fieldCoords.y = atof(y.c_str());
			}
			std::string r_id;
			do {
				std::string x, y, a;
				ss >> r_id >> x >> y;
				int _id = atoi(r_id.c_str());
				if (_id != id) {
					robots[_id].fieldCoords.x = atof(x.c_str());
					robots[_id].fieldCoords.y = atof(y.c_str());
				}


			} while (r_id != "99");
		}

	}
	if (command == "POS") {
		ss >> r_id;
		int _id = atoi(r_id.c_str());
		if (_id != id) {
			std::string x, y, a;
			ss >> x >> y >> a;
			robots[_id].fieldCoords.x = atoi(x.c_str());
			robots[_id].fieldCoords.y = atoi(y.c_str());
			robots[_id].polarMetricCoords.y = atoi(a.c_str());
		}
	}
}