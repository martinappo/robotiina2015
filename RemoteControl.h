#pragma once
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
class Robot;
/*
* UDP server for controlling robot
* use http://sourceforge.net/projects/sockettest/ to send commands
* see  RemoteControl::loop() for port number
* */
class RemoteControl {
public:
    RemoteControl( boost::asio::io_service &io, Robot * robot);
    void Start();
    void Stop();
    void loop();
protected:
    std::string respond(const std::string &query);

private:
	boost::asio::ip::udp::socket socket;
    boost::atomic<bool> stop;
    Robot *robot;
    boost::asio::io_service &io;
    boost::thread_group threads;

};