#pragma once
#include "types.h"
#include "ObjectPosition.h"
#include "BallPosition.h"
#include "GatePosition.h"
#include "RobotPosition.h"
#include "TargetPosition.h"

class FieldState;
class BallArray {
public:
	BallArray(unsigned ballCount, FieldState* field):field(field){
		balls.resize(ballCount);
		// distribute balls uniformly
		for (unsigned i = 0; i < ballCount; i++) {
			balls[i].id = i;
		}
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
	const BallPosition& getClosest(){
		return closest;
	}
	const BallPosition& calcClosest(int * index);

	size_t size() {
		return balls.size();
	}
public:
	BallPosition closest = BallPosition(true);
private:
	std::vector<BallPosition> balls;
	double ballLost = -1;
	FieldState *field;
	std::atomic_bool reset;
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
		/* our states */
		GAME_MODE_IN_PROGRESS,
		GAME_MODE_TAKE_BALL, // other robot passed pall
	};
	enum RobotColor {
		ROBOT_COLOR_YELLOW_UP = 0,
		ROBOT_COLOR_BLUE_UP
	};
	std::atomic_int gameMode;
	RobotColor robotColor = ROBOT_COLOR_BLUE_UP;
	std::atomic_bool collisionWithBorder;
	std::atomic_bool collisionWithUnknown;
	FieldState(const int number_of_balls);
	virtual ~FieldState();
	//BallPosition balls[number_of_balls]; //All others are distance from self and heading to it
	BallArray balls;
	GatePosition blueGate;
	GatePosition yellowGate;
	RobotPosition self; //Robot distance on field
	ObjectPosition partner;
	ObjectPosition opponents[2];
	ObjectPosition partnerHomeGate;
	std::atomic_bool gateObstructed;
	virtual void SetTargetGate(OBJECT gate) = 0;
	virtual GatePosition &GetTargetGate() = 0;
	virtual GatePosition &GetHomeGate() = 0;
	void resetBallsUpdateState();
	virtual void Lock() {};
	virtual void UnLock() {};
	virtual void SendMessage(const std::string message){};
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