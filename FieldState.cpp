#include "FieldState.h"

const void BallArray::updateAndFilterClosest(cv::Point2i closestRaw) {
	if (reset == true) {
		closest.filteredRawCoords = closestRaw;
		closest.lastRawCoords = closestRaw;
		closest.rawPixelCoords = closestRaw;
	}

	double distance = cv::norm(closestRaw - closest.lastRawCoords);
	if (distance > 20) { // another ball found
		double t2 = (double)cv::getTickCount();
		double dt = (t2 - ballLost) / cv::getTickFrequency();
		if (dt < 1.5) {
			//closest.predictCoords();
			closest.filteredRawCoords = closest.lastRawCoords;
			closest.rawPixelCoords = closest.lastRawCoords;
			closest.updateRawCoordinates(closest.filteredRawCoords);
			return;
		}
		else {
			reset = true;
		}
	}

	closest.rawPixelCoords = closestRaw; //Filter needs the raw coords to be set
	closest.filterCoords(closest, reset); //Sets filtered raw coords
	closest.updateRawCoordinates(closest.filteredRawCoords); //Update all coordinates after filtering raw ones
	closest.lastRawCoords = closestRaw;
	if (reset) {
		reset = false;
	}
	closest.isUpdated = true;
	ballLost = (double)cv::getTickCount();
}

FieldState::FieldState(int number_of_balls) :yellowGate(YELLOW_GATE), blueGate(BLUE_GATE), self(yellowGate, blueGate, cv::Point(0, 0)), balls(number_of_balls, this){
	gameMode = GAME_MODE_START_SINGLE_PLAY;
	collisionWithBorder = false;
	collisionWithUnknown = false;
	collisionRange = {0,0};
	isPlaying = false;
};
FieldState::~FieldState(){

}

void FieldState::resetBallsUpdateState() {
	for (size_t i = 0, isize = balls.size(); i < isize; i++) {
		balls[i].setIsUpdated(false);
	}
}
