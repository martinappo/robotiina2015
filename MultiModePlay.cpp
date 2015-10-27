#include "MultiModePlay.h"
#include "SingleModePlay.h"

class MasterModeIdle : public Idle {

	virtual DriveMode step(double dt) {
		switch (m_pFieldState->gameMode) {
		case FieldState::GAME_MODE_START_OPPONENT_KICK_OFF:
		case FieldState::GAME_MODE_START_OPPONENT_THROWIN:
		case FieldState::GAME_MODE_START_OPPONENT_FREE_KICK:
		case FieldState::GAME_MODE_START_OUR_KICK_OFF:
		case FieldState::GAME_MODE_START_OUR_FREE_KICK:
			m_pFieldState->gameMode = FieldState::GAME_MODE_IN_PROGRESS;
		case FieldState::GAME_MODE_START_OUR_THROWIN:
			return DRIVEMODE_DRIVE_TO_BALL;
		}
		return DRIVEMODE_IDLE;
	}
};
class SlaveModeIdle : public Idle {

	virtual DriveMode step(double dt) {
		switch (m_pFieldState->gameMode) {
		case FieldState::GAME_MODE_START_OPPONENT_KICK_OFF:
		case FieldState::GAME_MODE_START_OPPONENT_THROWIN:
		case FieldState::GAME_MODE_START_OPPONENT_FREE_KICK:
		case FieldState::GAME_MODE_START_OUR_KICK_OFF:
		case FieldState::GAME_MODE_START_OUR_FREE_KICK:
			m_pFieldState->gameMode = FieldState::GAME_MODE_IN_PROGRESS;
			return DRIVEMODE_DEFEND;
		case FieldState::GAME_MODE_START_OUR_THROWIN:
			return DRIVEMODE_DRIVE_TO_BALL;
		}
		return DRIVEMODE_IDLE;
	}
};
class Defend : public DriveInstruction {
public:
	Defend() : DriveInstruction("DEFEND"){};
	virtual DriveMode step(double dt) {
		return DRIVEMODE_DEFEND;
	}
};

std::pair<DriveMode, DriveInstruction*> MasterDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new MasterModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
};

std::pair<DriveMode, DriveInstruction*> SlaveDriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new SlaveModeIdle()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new Defend()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<DriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
};

MultiModePlay::MultiModePlay(ICommunicationModule *pComModule, FieldState *pState, bool bMaster) :StateMachine(pComModule, pState,
	bMaster ? TDriveModes(MasterDriveModes, MasterDriveModes + sizeof(MasterDriveModes) / sizeof(MasterDriveModes[0]))
	: TDriveModes(SlaveDriveModes, SlaveDriveModes + sizeof(SlaveDriveModes) / sizeof(SlaveDriveModes[0])))
	, isMaster(bMaster)
{
}


MultiModePlay::~MultiModePlay()
{
}
