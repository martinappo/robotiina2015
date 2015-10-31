#include "RefereeCom.h"


RefereeCom::RefereeCom(const std::string &name): ConfigurableModule(name)
{
	AddSetting("Field", [this]{return std::string(1,this->FIELD_MARKER);}, [this]{this->nextField();});
	AddSetting("Team", [this]{return std::string(1,this->TEAM_MARKER);}, [this]{this->nextTeam();});
	AddSetting("Robot", [this]{return std::string(1,this->ROBOT_MARKER);}, [this]{this->nextRobot();});
	LoadSettings();
}

bool RefereeCom::isCommandAvailable() {
	return !commandQueue.empty();
}

REFCOMMAND RefereeCom::getNextCommand() {
	REFCOMMAND nextCommand = commandQueue.front();
	commandQueue.pop();
	return nextCommand;
}

void RefereeCom::giveCommand(REFCOMMAND command) {
	commandQueue.push(command);
}

void RefereeCom::nextField() {
	if (FIELD_MARKER == 'A') {
		FIELD_MARKER = 'B';
	}
	else {
		FIELD_MARKER = 'A';
	}
}

void RefereeCom::nextTeam() {
	if (TEAM_MARKER == 'A') {
		TEAM_MARKER = 'B';
	}
	else {
		TEAM_MARKER = 'A';
	}
}

void RefereeCom::nextRobot() {
	if (ROBOT_MARKER == 'A') {
		ROBOT_MARKER = 'B';
	}
	else if (ROBOT_MARKER == 'B') {
		ROBOT_MARKER = 'C';
	}
	else if (ROBOT_MARKER == 'C') {
		ROBOT_MARKER = 'D';
	}
	else if (ROBOT_MARKER == 'D') {
		ROBOT_MARKER = 'A';
	}
}

/**********************************
* HARDWARE RECEIVER IMPLEMENTATION
***********************************/
LLAPReceiver::LLAPReceiver(boost::asio::io_service &io_service, std::string port, unsigned int baud_rate, const std::string &name)
	: RefereeCom(name), SimpleSerial(io_service, port, baud_rate), ThreadedClass(name) {}

LLAPReceiver::~LLAPReceiver()
{
	WaitForStop();
}

void LLAPReceiver::Run() {
	std::cout << "Referee listener starting" << std::endl;
	std::string command;
	while (!stop_thread){
		command = readNumberOfCharsAsync(12);
		if (command.length() == 12 && command.at(0) == 'a' && command.at(1) == FIELD_MARKER && (command.at(2) == ALL_MARKER || command.at(2) == ROBOT_MARKER)) {
			if (command.at(2) == ROBOT_MARKER) writeString("a" + std::string(1, FIELD_MARKER) + std::string(1, ROBOT_MARKER) + "ACK------");
			command = command.substr(3);
			if (command == "START----") commandQueue.push(START);
			else if (command == "STOP-----") commandQueue.push(STOP);
			else if (command == "PLACEDBAL") commandQueue.push(PLACED_BALL);
			else if (command == "ENDHALF--") commandQueue.push(END_HALF);
			else if (command.at(0) == TEAM_MARKER) {
				command = command.substr(1);
				if (command == "KICKOFF-") commandQueue.push(KICK_OFF);
				else if (command == "IFREEK--") commandQueue.push(INDIRECT_FREE_KICK);
				else if (command == "DFREEK--") commandQueue.push(DIRECT_FREE_KICK);
				else if (command == "GOALK---") commandQueue.push(GOAL_KICK);
				else if (command == "THROWIN-") commandQueue.push(THROW_IN);
				else if (command == "CORNERK-") commandQueue.push(CORNER_KICK);
				else if (command == "PENALTY-") commandQueue.push(PENALTY);
				else if (command == "GOAL----") commandQueue.push(GOAL);
				else if (command == "CARDY---") commandQueue.push(YELLOW_CARD);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50)); // do not poll serial to fast
	}
	std::cout << "Referee listener stoping" << std::endl;
}