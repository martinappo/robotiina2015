#pragma once
#include "types.h"
#include <boost/asio.hpp>

class ComPortScanner
{
private:
#ifdef WIN32
	const std::string prefix = "COM";
#else
	const std::string prefix = "/dev/ttyACM";
#endif

	std::string CheckPort(boost::asio::io_service &io_service, const std::string &port);

public:
	ComPortScanner();
	bool VerifyObject(boost::asio::io_service &io_service, const std::string &conf_file, int id);

	bool VerifyWheels(boost::asio::io_service &io_service, const std::string &conf_file = "conf/ports.ini");
	bool VerifyCoilboard(boost::asio::io_service &io_service, const std::string &conf_file = "conf/ports.ini");
	bool VerifyAll(boost::asio::io_service &io_service, const std::string &conf_file = "conf/ports.ini");

	bool Scan(boost::asio::io_service &io_service);
	bool ScanObject(boost::asio::io_service &io_service, const std::string &conf_file, int id);
	~ComPortScanner();
};

