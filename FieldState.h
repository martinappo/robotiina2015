#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "BallPosition.h"
#include "GatePosition.h"
#include "RobotPosition.h"
#include "TargetPosition.h"

class BallArray {
public:
	BallArray(unsigned ballCount){
		balls.resize(ballCount);
	}
	BallPosition& operator[](unsigned j) {
		return balls[j];
	}
	std::vector<BallPosition>::iterator begin() {
		return balls.begin();
	}
	std::vector<BallPosition>::iterator end() {
		return balls.end();
	}
private:
	std::vector<BallPosition> balls;
};
class FieldState {
public:
	enum GameMode {
		GAME_MODE_STOPED = 0,
		GAME_MODE_START_SINGLE_PLAY,

		GAME_MODE_PLACED_BALL,
		GAME_MODE_END_HALF,

		GAME_MODE_START_OUR_KICK_OFF,
		GAME_MODE_START_OPPONENT_KICK_OFF,

		GAME_MODE_START_OUR_THROWIN,
		GAME_MODE_START_OPPONENT_THROWIN,

		GAME_MODE_START_OUR_FREE_KICK,
		GAME_MODE_START_OPPONENT_FREE_KICK,

		GAME_MODE_START_OUR_INDIRECT_FREE_KICK,
		GAME_MODE_START_OPPONENT_INDIRECT_FREE_KICK,

		GAME_MODE_START_OUR_GOAL_KICK,
		GAME_MODE_START_OPPONENT_GOAL_KICK,

		GAME_MODE_START_OUR_CORNER_KICK,
		GAME_MODE_START_OPPONENT_CORNER_KICK,

		GAME_MODE_START_OUR_PENALTY,
		GAME_MODE_START_OPPONENT_PENALTY,

		GAME_MODE_START_OUR_GOAL,
		GAME_MODE_START_OPPONENT_GOAL,

		GAME_MODE_START_OUR_YELLOW_CARD,
		GAME_MODE_START_OPPONENT_YELLOW_CARD,

		GAME_MODE_IN_PROGRESS
	};
	std::atomic_int gameMode;
	FieldState();
	virtual ~FieldState();
	BallPosition balls[NUMBER_OF_BALLS]; //All others are distance from self and heading to it
	//BallArray balls = BallArray(NUMBER_OF_BALLS);
	GatePosition blueGate;
	GatePosition yellowGate;
	RobotPosition self; //Robot distance on field
	ObjectPosition partner;
	ObjectPosition opponents[2];
	std::atomic_bool gateObstructed;
	virtual void SetTargetGate(OBJECT gate) = 0;
	virtual GatePosition &GetTargetGate() = 0;
	virtual GatePosition &GetHomeGate() = 0;
	void resetBallsUpdateState();
	virtual void Lock() {};
	virtual void UnLock() {};

};

class FieldStateLock{
public:
	FieldStateLock(FieldState * pFieldState){
		m_pState = pFieldState;
		m_pState->Lock();
	}
	~FieldStateLock(){
		m_pState->UnLock();
	}
	FieldState * operator ->(){
		return m_pState;
	}
private:
	FieldState * m_pState;
};