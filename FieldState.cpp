#include "FieldState.h"

const void BallArray::updateAndFilterClosest(cv::Point2i possibleClosestRaw, std::vector<cv::Point2i> rawBallCoords, bool ballIsNotValid) {
	if (reset == true || ballIsNotValid) { 
		closest.filteredRawCoords = possibleClosestRaw;
		closest.lastRawCoords = possibleClosestRaw;
		closest.rawPixelCoords = possibleClosestRaw;
	}

	double distance = cv::norm(possibleClosestRaw - closest.lastRawCoords);
	if (distance > 20) { // another ball found
		//detect, if correct ball is in vector
		bool foundFromVector = false;
		for (auto rawBallCoord : rawBallCoords) { //Sorted
			if (cv::norm(rawBallCoord - closest.lastRawCoords) <= 40) {
				possibleClosestRaw = rawBallCoord;
				foundFromVector = true;
				break;
			}
		}

		if (!foundFromVector) {
			double t2 = (double)cv::getTickCount();
			double dt = (t2 - ballLost) / cv::getTickFrequency();
			if (dt < 1) {
				closest.predictCoords();
				//closest.filteredRawCoords = closest.lastRawCoords;
				closest.rawPixelCoords = possibleClosestRaw;
				closest.updateRawCoordinates(closest.filteredRawCoords);
				closest.lastRawCoords = closest.filteredRawCoords;
				return;
			}
			else {
				reset = true;
			}
		}
	}

	closest.rawPixelCoords = possibleClosestRaw; //Filter needs the raw coords to be set
	closest.filterCoords(closest, reset); //Sets filtered raw coords
	closest.updateRawCoordinates(closest.filteredRawCoords); //Update all coordinates after filtering raw ones
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
