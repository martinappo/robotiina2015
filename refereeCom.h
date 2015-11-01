#pragma once
#include "SimpleSerial.h"
#include "ThreadedClass.h"
#include "ConfigurableModule.h"
#include <queue>
#include "FieldState.h"

class RefereeCom: public ConfigurableModule
{
public:
	RefereeCom(FieldState *pFieldState, const std::string &name = "Referee");
	void giveCommand(FieldState::GameMode command);

	virtual bool isTogglable() { return false; }
	void setField(FieldState *pFieldState){ m_pFieldState = pFieldState; }
protected:
	FieldState * m_pFieldState = NULL;

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
	LLAPReceiver(FieldState *pFieldState, boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200, const std::string &name = "Referee");
	~LLAPReceiver();

	bool isTogglable() { return true; }
	void handleMessage(const std::string & message);
	virtual void messageReceived(const std::string & message);

protected: 
	void Run();

};

