#include "FieldState.h"

FieldState::FieldState() :yellowGate(YELLOW_GATE), blueGate(BLUE_GATE), self(yellowGate, blueGate, cv::Point(0, 0)){
	std::cout << 1 << std::endl;
};
FieldState::~FieldState(){

}

void FieldState::resetBallsUpdateState() {
	for (int i = 0; i < NUMBER_OF_BALLS; i++) {
		balls[i].setIsUpdated(false);
	}
}
