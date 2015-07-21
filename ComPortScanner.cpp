#include "ComPortScanner.h"
#include "SimpleSerial.h"
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>

ComPortScanner::ComPortScanner()
{
}

bool ComPortScanner::VerifyObject(boost::asio::io_service &io_service, const std::string &conf_file, int id) 
{
	bool ok = true;
	boost::property_tree::ptree ports;
	try {
		std::cout << "conf/" << conf_file << ".ini"<<std::endl;
		read_ini("conf/"+conf_file+".ini", ports);
	}
	catch (...) {
		std::cout << "Error reading old port configuration: " << std::endl;
		return false;
	}

	std::stringstream portNum;
	std::string _id = std::to_string(id); // v.first;

	try {
		portNum << ports.get<std::string>(std::to_string(id));//(v.second).data();
	}
	catch (...)
	{
		std::cout << "ID: " << _id << " not found in conf file" << std::endl;
		ok = false;
	}

	std::string id2 = CheckPort(io_service, portNum.str());
	if (id2.empty()) {
		ok = false;
	}
	
	return ok;
}


bool ComPortScanner::VerifyWheels(boost::asio::io_service &io_service, const std::string &conf_file)
{
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
}

bool ComPortScanner::VerifyCoilboard(boost::asio::io_service &io_service, const std::string &conf_file)
{
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
}

bool ComPortScanner::VerifyAll(boost::asio::io_service &io_service, const std::string &conf_file) {
	return VerifyCoilboard(io_service, conf_file) && VerifyWheels(io_service, conf_file);
}

bool ComPortScanner::Scan(boost::asio::io_service &io_service)
{
//	std::map<short, std::string> portMap;
	boost::property_tree::ptree ports;
	for (int i = 0; i < 20; i++) {
		std::stringstream portNum;
		portNum << prefix << i;
		std::string id = CheckPort(io_service, portNum.str());
		if (!id.empty()) {
			ports.put(id, portNum.str());
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

bool ComPortScanner::ScanObject(boost::asio::io_service &io_service, const std::string &conf_file, int id)
{
//	std::map<short, std::string> portMap;
	boost::property_tree::ptree ports;


	bool ok = true;
	for (int i = 0; i < 20; i++) {
		std::stringstream portNum;
		portNum << prefix << i;
		std::string id = CheckPort(io_service, portNum.str());
		if (!id.empty()) {
			ports.put(id, portNum.str());
		}
	}
	if (true){
		write_ini("conf/"+conf_file+"_new.ini", ports);
	}
	if (VerifyObject(io_service, conf_file+"_new", id)) {
#ifdef WIN32
		_unlink(("conf/"+conf_file+".ini").c_str());
#else
		unlink(("conf/"+conf_file+".ini").c_str());
#endif
		std::cout <<  ("conf/"+conf_file+".ini")<<std::endl;
		std::cout << ("conf/"+conf_file+"_new.ini")<<std::endl;
		rename(("conf/"+conf_file+"_new.ini").c_str(), ("conf/"+conf_file+".ini").c_str());
		return true;
	}
	return false;



}

std::string ComPortScanner::CheckPort(boost::asio::io_service &io_service, const std::string &portNum)
{
	try {
		std::string id = "";
		SimpleSerial port = SimpleSerial(io_service, portNum, 115200);
		for (int i = 0; i < 10; i++) {
			port.writeString("?\n");
			id = port.readLineAsync(1000);

			std::cout << "Check result: " << portNum << " -> " << id << std::endl;
			if (id == "discharged") continue;
			if (id == "~x~~x~?") continue;
			if (id.empty()) continue;
			if (id.substr(0, 4) == "<sta") continue;
			if (id == "<id:0>") throw std::runtime_error(("ID not set in port " + portNum).c_str());
			if (id.substr(0, 4) != "<id:") continue; //throw std::runtime_error(("Invalid ID " + id + " received from port " + portNum).c_str());

			id = id.substr(4, 1);
			std::cout << "Found port " << portNum << ", id: " << id << std::endl;
			return id;
		}
		if (id.empty()) throw std::runtime_error(("No ID received from port " + portNum).c_str());
	}
	catch (std::runtime_error const&e) {
		std::cout << "Port not accessible: " << portNum << ", error: " << e.what() << std::endl;
	}
	return "";
}

ComPortScanner::~ComPortScanner()
{
}
