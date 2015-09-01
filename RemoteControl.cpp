#include "types.h"
#include "Robot.h"
#include "RemoteControl.h"
#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

using boost::asio::ip::udp;
RemoteControl::RemoteControl(boost::asio::io_service &io, ICommunicationModule *pComModule) :io(io), m_pComModule(pComModule)
, socket(io, udp::endpoint(udp::v4(), 10004)) 
{

}


std::string RemoteControl::respond(const std::string &query)
{
	std::string command(query.substr(0, query.find('#')));
	std::cout << "Remote control got command: " << command << std::endl;

	return RemoteControl::ExecuteRemoteCommand(command);
}

std::string RemoteControl::ExecuteRemoteCommand(const std::string &command){
	std::stringstream response;
	//boost::mutex::scoped_lock lock(remote_mutex); //allow one command at a time
	std::vector<std::string> tokens;
	boost::split(tokens, command, boost::is_any_of(";"));
	std::string query = tokens[0];
	/*
	STATE s = (STATE)GetState();
	if (query == "status") response << s;
	else if (query == "remote") SetState(STATE_REMOTE_CONTROL);
	else if (query == "cont") SetState(STATE_RUN);
	else if (query == "reset") SetState(STATE_NONE);
	else if (query == "exit") SetState(STATE_END_OF_GAME);
	else if (STATE_REMOTE_CONTROL == s) {*/
		if (query == "drive" && tokens.size() == 3) {
			int speed = atoi(tokens[1].c_str());
			double direction = atof(tokens[2].c_str());
			m_pComModule->Drive(speed, direction, 0);
		}
		else if (query == "rdrive" && tokens.size() == 4) {
			int speed = atoi(tokens[1].c_str());
			double direction = atof(tokens[2].c_str());
			int rotate = atoi(tokens[3].c_str());
			m_pComModule->Drive(speed, direction, rotate);
		}

	//}
		response << "ok: " << command;
	return response.str();
}

void RemoteControl::Run()
{
	std::cout << "Remote control is starting " << std::endl;
	try {
	while (!stop_thread)
    {
        boost::array<char, 1024> recv_buf;
        udp::endpoint remote_endpoint;
        boost::system::error_code error;
        socket.receive_from(boost::asio::buffer(recv_buf),
                remote_endpoint, 0, error);

		if (error && error != boost::asio::error::message_size) {
			std::cout << "Remote control is terminating: " << error.message() << std::endl;
			stop_thread = true;
			return; // throw boost::system::system_error(error);
		}

        std::string message = respond(std::string(recv_buf.begin(), recv_buf.end()));

        boost::system::error_code ignored_error;
        socket.send_to(boost::asio::buffer(message),
                remote_endpoint, 0, ignored_error);
    }
} catch(...){
     std::cout << "Remote control is terminating" << std::endl;
}
	std::cout << "Remote control is exiting " << std::endl;

}


