#pragma once
#include "types.h"
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
	void handleMessage(const std::string & message);
	virtual void sendAck(const std::string & message){};
	const char ALL_MARKER = 'X';
public:
	char FIELD_MARKER = 'A';
	char TEAM_MARKER = 'A';
	char ROBOT_MARKER = 'A';
private:
	void nextField();
	void nextTeam();
	void nextRobot();
};

class LLAPReceiver : public RefereeCom, public SimpleSerial
{
public:
	LLAPReceiver(FieldState *pFieldState, boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200, const std::string &name = "Referee");
	~LLAPReceiver();

	bool isTogglable() { return true; }
	virtual void DataReceived(const std::string & message);
	virtual void sendAck(const std::string & message){
		WriteString(message);
	}
};

