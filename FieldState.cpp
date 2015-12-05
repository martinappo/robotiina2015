#include "FieldState.h"

const void BallArray::updateAndFilterClosest(cv::Point2i possibleClosestRaw, std::vector<cv::Point2i> rawBallCoords, bool ballIsNotValid, bool filterEnabled) {

	if (!filterEnabled) {
		closest.updateRawCoordinates(possibleClosestRaw);
		closest.lastRawCoords = possibleClosestRaw;
		closest.isUpdated = true;
		return;
	}

	if (reset == true || ballIsNotValid) { 
		closest.lastRawCoords = possibleClosestRaw;
		closest.rawPixelCoords = possibleClosestRaw;
		closest.updateRawCoordinates(possibleClosestRaw);
	}

	double distance = cv::norm(possibleClosestRaw - closest.lastRawCoords);
	if (distance > 40) { // another ball found
		//detect, if correct ball is in vector
		bool foundFromVector = false;
		for (auto rawBallCoord : rawBallCoords) { //Sorted
			if (cv::norm(rawBallCoord - closest.lastRawCoords) <= 50) {
				possibleClosestRaw = rawBallCoord;
				foundFromVector = true;
				break;
			}
		}

		if (!foundFromVector) {
			double t2 = (double)cv::getTickCount();
			double dt = (t2 - ballLost) / cv::getTickFrequency();
			if (dt < 1) {
				//closest.filteredRawCoords = closest.lastRawCoords;
				closest.rawPixelCoords = possibleClosestRaw;
				closest.updateRawCoordinates(possibleClosestRaw);
				closest.predictCoords();
				//closest.lastRawCoords = closest.filteredRawCoords;
				return;
			}
			else {
				reset = true;
			}
		}
	}

	closest.rawPixelCoords = possibleClosestRaw; 
	closest.updateRawCoordinates(possibleClosestRaw);
	closest.filterCoords(closest, reset); //Sets polar
	closest.lastRawCoords = possibleClosestRaw;
	if (reset) {
		reset = false;
	}
	closest.isUpdated = true;
	ballLost = (double)cv::getTickCount();
}

FieldState::FieldState(int number_of_balls) :yellowGate(YELLOW_GATE), blueGate(BLUE_GATE), self(yellowGate, blueGate, cv::Point(0, 0)), balls(number_of_balls, this), opponents(2, this) {
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
