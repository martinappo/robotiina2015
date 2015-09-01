#pragma once
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include "types.h"
#include "ThreadedClass.h"

/*
* UDP server for controlling robot
* use http://sourceforge.net/projects/sockettest/ to send commands
* see  RemoteControl::loop() for port number
* */
class RemoteControl: public ThreadedClass, public IControlModule  {
public:
	RemoteControl(boost::asio::io_service &io, ICommunicationModule *pComModule);
	void Run();

protected:
    std::string respond(const std::string &query);
	std::string ExecuteRemoteCommand(const std::string &command);

private:
	boost::asio::ip::udp::socket socket;
    boost::atomic<bool> stop;
	ICommunicationModule *m_pComModule;
	boost::asio::io_service &io;

};