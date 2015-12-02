#include "refereeCom.h"


RefereeCom::RefereeCom(FieldState *pFieldState, const std::string &name) : ConfigurableModule(name), m_pFieldState(pFieldState)
{
	AddSetting("Field", [this]{return std::string(1,this->FIELD_MARKER);}, [this]{this->nextField();});
	AddSetting("Team", [this]{return std::string(1,this->TEAM_MARKER);}, [this]{this->nextTeam();});
	AddSetting("Robot", [this]{return std::string(1,this->ROBOT_MARKER);}, [this]{this->nextRobot();});
	LoadSettings();
}

void RefereeCom::giveCommand(FieldState::GameMode command) {
	m_pFieldState->gameMode = command;
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
void RefereeCom::handleMessage(const std::string & message){
	//TODO: update m_pFieldState->gameMode from here directly, add missing start commands there
	if (m_pFieldState == NULL) return;
	std::cout << "referee command: " << message << std::endl;
	std::string command = message.substr(0, 12);
	if (command.length() == 12 && command.at(0) == 'a' && command.at(1) == FIELD_MARKER && (command.at(2) == ALL_MARKER || command.at(2) == ROBOT_MARKER)) {
		if (command.at(2) == ROBOT_MARKER) sendAck("a" + std::string(1, FIELD_MARKER) + std::string(1, ROBOT_MARKER) + "ACK------");
		command = command.substr(3);
		if (command == "START----") m_pFieldState->gameMode = FieldState::GAME_MODE_START_SINGLE_PLAY;
		else if (command == "STOP-----") m_pFieldState->gameMode = FieldState::GAME_MODE_STOPED;
		else if (command == "PLACEDBAL") m_pFieldState->gameMode = FieldState::GAME_MODE_PLACED_BALL;
		else if (command == "ENDHALF--") m_pFieldState->gameMode = FieldState::GAME_MODE_END_HALF;
		else if (command.at(0) == TEAM_MARKER) {
			command = command.substr(1);
			if (command == "KICKOFF-") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_KICK_OFF;
			else if (command == "IFREEK--") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_INDIRECT_FREE_KICK;
			else if (command == "DFREEK--") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_FREE_KICK;
			else if (command == "GOALK---") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_GOAL_KICK;
			else if (command == "THROWIN-") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_THROWIN;
			else if (command == "CORNERK-") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_CORNER_KICK;
			else if (command == "PENALTY-") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_PENALTY;
			else if (command == "GOAL----") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_GOAL;
			else if (command == "CARDY---") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OUR_YELLOW_CARD;
		}
		else {
			command = command.substr(1);
			if (command == "KICKOFF-") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_KICK_OFF;
			else if (command == "IFREEK--") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_INDIRECT_FREE_KICK;
			else if (command == "DFREEK--") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_FREE_KICK;
			else if (command == "GOALK---") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_GOAL_KICK;
			else if (command == "THROWIN-") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_THROWIN;
			else if (command == "CORNERK-") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_CORNER_KICK;
			else if (command == "PENALTY-") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_PENALTY;
			else if (command == "GOAL----") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_GOAL;
			else if (command == "CARDY---") m_pFieldState->gameMode = FieldState::GAME_MODE_START_OPPONENT_YELLOW_CARD;
		}
	}
}

/**********************************
* HARDWARE RECEIVER IMPLEMENTATION
***********************************/
LLAPReceiver::LLAPReceiver(FieldState *pFieldState, boost::asio::io_service &io_service, std::string port, unsigned int baud_rate, const std::string &name)
	: RefereeCom(pFieldState, name), SimpleSerial(io_service, port, baud_rate) {}

LLAPReceiver::~LLAPReceiver()
{
}
void LLAPReceiver::DataReceived(const std::string & message){
	handleMessage(message);
};
