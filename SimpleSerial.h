#pragma  once
//#include "types.h"
#include "blockingreader.h"
#include <boost/asio.hpp>

class SimpleSerial {
public:

	SimpleSerial(boost::asio::io_service &io_service, const std::string & port, unsigned int baud_rate) : io(io_service), serial(io, port) {
		serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
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

protected:
	boost::asio::io_service &io;
	boost::asio::serial_port serial;
};
