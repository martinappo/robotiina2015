#pragma once
#include "SimpleSerial.h"
#include "ThreadedClass.h"
#include "ConfigurableModule.h"
#include "types.h"
#include <queue>

class RefereeCom: public SimpleSerial, public ThreadedClass, public ConfigurableModule
{
public:
	RefereeCom(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200, const std::string &name = "Referee");
	~RefereeCom();
	const char ALL_MARKER = 'X';
	char FIELD_MARKER = 'A';
	char TEAM_MARKER = 'A';
	char ROBOT_MARKER = 'A';

	bool isCommandAvailable();
	REFCOMMAND getNextCommand();

protected:
	void Run();

private:
	std::queue<REFCOMMAND> commandQueue;
	void nextField();
	void nextTeam();
	void nextRobot();
};

