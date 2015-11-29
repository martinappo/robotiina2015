#include "FieldState.h"

FieldState::FieldState(int number_of_balls) :yellowGate(YELLOW_GATE), blueGate(BLUE_GATE), self(yellowGate, blueGate, cv::Point(0, 0)), balls(number_of_balls){
	gameMode = GAME_MODE_STOPED;
};
FieldState::~FieldState(){

}

void FieldState::resetBallsUpdateState() {
	for (size_t i = 0; i < balls.size(); i++) {
		balls[i].setIsUpdated(false);
	}
}
