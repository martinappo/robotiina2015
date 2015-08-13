#pragma once
#include "types.h"
#include <boost/asio.hpp>
#include "SimpleSerial.h"
#include <atomic>
#include "ThreadedClass.h"
class ComPortScanner: public ThreadedClass
{
private:
#ifdef WIN32
	const std::string prefix = "COM";
#else
	const std::string prefix = "/dev/ttyACM";
#endif
	boost::asio::io_service &io_service;
	int CheckPort(const std::string &port, /*out*/ SimpleSerial **pPort);
	std::map<std::string, std::pair<int, SimpleSerial*>> m_mPorts; // port name => <id, open port>
	//std::vector<std::tuple<const std::string, std::vector<int>, std::function<void()>>> m_vScanQueue;
protected:
	void Run();
public:
	ComPortScanner(boost::asio::io_service &io);
	bool VerifyPort(const std::string &portNum, int id, /*out*/ SimpleSerial **pPort);

	bool VerifyWheels(boost::asio::io_service &io_service, const std::string &conf_file = "conf/ports.ini");
	bool VerifyCoilboard(boost::asio::io_service &io_service, const std::string &conf_file = "conf/ports.ini");
	bool VerifyAll(boost::asio::io_service &io_service, const std::string &conf_file = "conf/ports.ini");

	bool Scan();
	bool ScanObject(const std::string &conf_file, std::vector<int> ids);
	~ComPortScanner();
};

