#include "types.h"
#include "robot.h"
#include "remotecontrol.h"
#include <sstream>
#include <iostream>

using boost::asio::ip::udp;
RemoteControl::RemoteControl(boost::asio::io_service &io, Robot * robot) :io(io), robot(robot)
, socket(io, udp::endpoint(udp::v4(), 10004)) 
{

}

void RemoteControl::Start()
{
    stop = false;

    threads.create_thread(boost::bind(&RemoteControl::loop, this));

}

void RemoteControl::Stop()
{
    std::cout << "Stoping Remote control " << std::endl;
    stop = true;
    socket.close();
    threads.join_all();
    std::cout << "Remote control stoped" << std::endl;
}

std::string RemoteControl::respond(const std::string &query)
{
    return robot->ExecuteRemoteCommand(query.substr(0, query.find('#')));
}

void RemoteControl::loop()
{

try {
    while (!stop)
    {
        boost::array<char, 1024> recv_buf;
        udp::endpoint remote_endpoint;
        boost::system::error_code error;
        socket.receive_from(boost::asio::buffer(recv_buf),
                remote_endpoint, 0, error);

		if (error && error != boost::asio::error::message_size) {
			std::cout << "Remote control is terminating: " << error.message() << std::endl;
			stop = true;
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
}


