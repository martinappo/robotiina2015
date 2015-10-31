#pragma once
#include "SimpleSerial.h"
#include "ThreadedClass.h"
#include "ConfigurableModule.h"
#include "types.h"
#include <queue>

class RefereeCom: public IRefereeCom, public ConfigurableModule
{
public:
	RefereeCom(const std::string &name = "Referee");
	bool isCommandAvailable();
	REFCOMMAND getNextCommand();
	void giveCommand(REFCOMMAND command);

	virtual bool isTogglable() { return false; }
protected:
	std::queue<REFCOMMAND> commandQueue;

	const char ALL_MARKER = 'X';
	char FIELD_MARKER = 'A';
	char TEAM_MARKER = 'A';
	char ROBOT_MARKER = 'A';
private:
	void nextField();
	void nextTeam();
	void nextRobot();
};

class LLAPReceiver : public RefereeCom, public SimpleSerial, public ThreadedClass
{
public:
	LLAPReceiver(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200, const std::string &name = "Referee");
	~LLAPReceiver();

	bool isTogglable() { return true; }
protected: 
	void Run();

};

