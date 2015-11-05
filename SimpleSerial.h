#pragma  once
//#include "types.h"
#include "blockingreader.h"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

class SimpleSerial {
public:

	SimpleSerial(boost::asio::io_service &io_service, const std::string & port, unsigned int baud_rate) : io(io_service), serial(io, port) {
		serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
		read_start();
	}
#ifdef WIN32	
	~SimpleSerial(){
		serial.close();
	}
#endif

	void writeString(std::string s)	{
		boost::asio::write(serial, boost::asio::buffer(s.c_str(), s.size()));
	}


	std::string readLine() {
		//Reading data char by char, code is optimized for simplicity, not speed
		using namespace boost;
		char c;
		std::string result;
		for (;;)
		{
			
			asio::read(serial, asio::buffer(&c, 1));
			
			switch (c)
			{
				case '\r':
					break;
				case '\n':
					return result;
				default:
					result += c;
			}
			
		}
	}
	std::string readLineAsync(size_t timeout = 50) {
		//Reading data char by char, code is optimized for simplicity, not speed
		using namespace boost;
		char c;
		std::string result;
		for (;;)
		{

			blockingreader reader(serial, timeout);

			while (reader.read_char(c) && c != '\n'){
				result += c;
			}
			if (c != '\n'){
				return result;
			}
			return result;

		}
	}

	std::string readNumberOfCharsAsync(int count, size_t timeout = 50) {
		using namespace boost;
		char c;
		std::string result;

		blockingreader reader(serial, timeout);

		while (reader.read_char(c) && count-- != 0){
			result += c;
		}
		return result;
	}
	virtual void messageReceived(const std::string & message){};
protected:
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
			//std::cout << "udp packet:" << message << std::endl;
			messageReceived(message);

			read_start(); // start waiting for another asynchronous read again
		}

	}


	boost::asio::io_service &io;
	boost::asio::serial_port serial;
	//char read_msg_[max_read_length]; // data read from the socket
	boost::array<char, max_read_length> read_msg_;

};
