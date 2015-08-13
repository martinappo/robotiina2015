#include "ComPortScanner.h"
#include "SimpleSerial.h"
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>

ComPortScanner::ComPortScanner(boost::asio::io_service &io) : io_service(io)
{
}

bool ComPortScanner::VerifyPort(const std::string &portNum, int id, /*out*/ SimpleSerial **pPort)
{
	if (running) return false; // port scanning in progress
	int id2 = CheckPort(portNum, pPort);
	return id == id2;
}


bool ComPortScanner::VerifyWheels(boost::asio::io_service &io_service, const std::string &conf_file)
{
	return true;
	/*
	bool ok = true;
	boost::property_tree::ptree ports;

	try {
		read_ini(conf_file, ports);
	}
	catch (...) {
		std::cout << "Error reading old port configuration: " << std::endl;
		return false;
	}

	for (int i = ID_WHEEL_FRONT_1; i < WHEEL_COUNT; i++) {
		// v.first is the name of the child.
		// v.second is the child tree.
		std::stringstream portNum;
		//portNum << prefix << 
		std::string _id = std::to_string(i); // v.first;
		try {
			portNum << ports.get<std::string>(std::to_string(i));//(v.second).data();
		}
		catch (...)
		{
			std::cout << "ID: " << _id << " not found in conf file" << std::endl;
			ok = false;
			continue;
		}

		std::string id = CheckPort(io_service, portNum.str());
		if (id.empty()) {
			ok = false;
		}
	}
	return ok;
	*/
}

bool ComPortScanner::VerifyCoilboard(boost::asio::io_service &io_service, const std::string &conf_file)
{
	return false;
	/*
	bool ok = true;
	boost::property_tree::ptree ports;

	try {
		read_ini(conf_file, ports);
	}
	catch (...) {
		std::cout << "Error reading old port configuration: " << std::endl;
		return false;
	}

	std::stringstream portNum;
	std::string _id = std::to_string(ID_COILGUN);
	try {
		portNum << ports.get<std::string>(std::to_string(ID_COILGUN));
	}
	catch (...)
	{
		std::cout << "ID: " << _id << " not found in conf file" << std::endl;
		ok = false;
	}

	std::string id = CheckPort(io_service, portNum.str());
	if (id.empty()) {
		ok = false;
	}

	return ok;
	*/
}

bool ComPortScanner::VerifyAll(boost::asio::io_service &io_service, const std::string &conf_file) {
	return VerifyCoilboard(io_service, conf_file) && VerifyWheels(io_service, conf_file);
}

bool ComPortScanner::Scan()
{
//	std::map<short, std::string> portMap;
	boost::property_tree::ptree ports;
	for (int i = 0; i < 20; i++) {
		std::stringstream portNum;
		portNum << prefix << i;
		SimpleSerial * pPort = NULL;
		int id = CheckPort(portNum.str(), &pPort);
		if (id > 0) {
			ports.put(std::to_string(id), portNum.str());
		}
	}
	write_ini("conf/ports_new.ini", ports);
	if (VerifyAll(io_service, "conf/ports_new.ini") ) {
#ifdef WIN32
		_unlink("conf/ports.ini");
#else
		unlink("conf/ports.ini");
#endif
		rename("conf/ports_new.ini", "conf/ports.ini");
		return true;
	}
	return false;
}

bool ComPortScanner::ScanObject(const std::string &conf_file, std::vector<int> ids)
{
	boost::property_tree::ptree ports;
	int found = 0;
	for (auto id : ids){
		for (auto port : m_mPorts) {
			if (port.second.first == id){
				found++;
				ports.put(std::to_string(id), port.first);
			}
		}
	}
	if (found == ids.size()) {
		write_ini(conf_file, ports);
		return true;
	}
	if (running) return false;
	Start();
	return false;
}

void ComPortScanner::Run() {
	for (int i = 9; i < 20; i++) {
		std::stringstream portNum;
		portNum << prefix << i;
		SimpleSerial * pPort = NULL;
		CheckPort(portNum.str(), &pPort);
	}
}

int ComPortScanner::CheckPort(const std::string &portNum, /*out*/ SimpleSerial **ppPort)
{
	std::string id = "";
	if (m_mPorts.find(portNum) != m_mPorts.end() && m_mPorts[portNum].second != NULL){
		if (m_mPorts[portNum].first > 0){
			*ppPort = m_mPorts[portNum].second;
			return m_mPorts[portNum].first;
		}//else
	}
	else {
		try {
			m_mPorts[portNum].second = new SimpleSerial(io_service, portNum, 115200);
			m_mPorts[portNum].first = -1;
		}
		catch (std::runtime_error const&e) {
			std::cout << "Port not accessible: " << portNum << ", error: " << e.what() << std::endl;
			m_mPorts[portNum].second = NULL;
			m_mPorts[portNum].first = -1;
			return false;
		}
	}
	
		
		for (int i = 0; i < 10; i++) {
			m_mPorts[portNum].second->writeString("?\n");
			id = m_mPorts[portNum].second->readLineAsync(1000);

			std::cout << "Check result: " << portNum << " -> " << id << std::endl;
			if (id == "discharged") continue;
			if (id == "~x~~x~?") continue;
			if (id.empty()) continue;
			if (id.substr(0, 4) == "<sta") continue;
			if (id == "<id:0>") throw std::runtime_error(("ID not set in port " + portNum).c_str());
			if (id.substr(0, 4) != "<id:") continue; //throw std::runtime_error(("Invalid ID " + id + " received from port " + portNum).c_str());

			id = id.substr(4, 1);
			std::cout << "Found port " << portNum << ", id: " << id << std::endl;
			int _id = atoi(id.c_str());
			m_mPorts[portNum].first = _id;
			*ppPort = m_mPorts[portNum].second;
			return _id;
		}
		if (id.empty()) {
			std::cout << "No ID received from port " << portNum << std::endl;
		}
	*ppPort = NULL;
	return 0;
}

ComPortScanner::~ComPortScanner()
{
	for (auto i : m_mPorts) {
		if (i.second.second != NULL) {
			delete i.second.second;
		}
		i.second.second = NULL;
	}

}
