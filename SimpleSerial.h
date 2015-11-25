#pragma  once
#include "types.h"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <mutex>
#include <chrono>
#include <thread>

//#define DUMP_SERIAL

#ifdef DUMP_SERIAL
#include <fstream>
#endif


class SimpleSerial: public ISerial {
protected:
#ifdef DUMP_SERIAL
	std::ofstream log;
#endif

public:

	SimpleSerial(boost::asio::io_service &io_service, const std::string & port, unsigned int baud_rate) : io(io_service), serial(io, port) {
		serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
		read_start();
#ifdef DUMP_SERIAL
		log.open("seral.log");
#endif

	}
	virtual ~SimpleSerial(){
#ifdef WIN32	
		serial.close();
#endif
#ifdef DUMP_SERIAL
		log.close();
#endif

	}

	void SendCommand(int id, const std::string &cmd, int param=INT_MAX)	{
		std::ostringstream oss;

		oss << id << ":" << cmd;
		if (param < INT_MAX) oss << param;
		oss << "\n";
		WriteString(oss.str());
	}
	void WriteString(const std::string &s)	{
		std::lock_guard<std::mutex> lock(writeLock);
		boost::asio::write(serial, boost::asio::buffer(s.c_str(), s.size()));
#ifdef DUMP_SERIAL
		log << s;
		log.flush();
#endif
//		std::chrono::milliseconds dura(50);
//		std::this_thread::sleep_for(dura);
 	}

	virtual void DataReceived(const std::string & message){};
	virtual void SetMessageHandler(ISerialListener* callback) {
		messageCallback = callback;
	};

protected:

	std::mutex writeLock;
	ISerialListener *messageCallback = NULL;
	static const int max_read_length = 512; // maximum amount of data to read in one operation

	void read_start(void)
	{ // Start an asynchronous read and call read_complete when it completes or fails
		serial.async_read_some(boost::asio::buffer(read_msg_, max_read_length),
			boost::bind(&SimpleSerial::read_complete,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void read_complete(const boost::system::error_code& error, size_t bytes_transferred)
	{ // the asynchronous read operation has now completed or failed and returned an error
		if (!error)
		{ // read completed, so process the data
			std::string message = std::string(read_msg_.begin(), read_msg_.end()).substr(0, bytes_transferred);
			//std::cout << "serial packet:" << message << std::endl;
			DataReceived(message);
			if (messageCallback != NULL) {
				messageCallback->DataReceived(message);
			}

			read_start(); // start waiting for another asynchronous read again
		}

	}


	boost::asio::io_service &io;
	boost::asio::serial_port serial;
	//char read_msg_[max_read_length]; // data read from the socket
	boost::array<char, max_read_length> read_msg_;

};
