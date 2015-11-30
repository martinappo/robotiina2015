#include "FieldState.h"
//#define PREDICT_CLOSEST_BALL

const BallPosition& BallArray::calcClosest(int * index){
	double alpha = 3;
	double target_distance = INT_MAX;
	int target_index = 0;
	for (unsigned i = 0; i < balls.size(); i++) {
		//if (abs(balls[i].fieldCoords.y) > 250) continue; // too far outside of the field
		double curDist = balls[i].getDistance();
		//if (includeHeading)
		//	curDist += alpha * curDist * sin(fabs(balls[i].getHeading()) / 180 * CV_PI);
		//std::cout << "getClosest: " << (includeHeading ? 1 : 0) << " " << balls[i].getDistance() << ", " << balls[i].getHeading() << " -> " << curDist << std::endl;
		if (curDist < target_distance) {
			target_index = i;
			target_distance = curDist;
		}
	}
	*index = target_index;
#ifndef PREDICT_CLOSEST_BALL
	closest.rawPixelCoords = balls[target_index].rawPixelCoords;
	closest.polarMetricCoords = balls[target_index].polarMetricCoords;
	closest.fieldCoords = balls[target_index].fieldCoords;
#else
	std::cout << balls[target_index].polarMetricCoords << ", " << closest.fieldCoords << ", " << balls[target_index].fieldCoords  << std::endl;
	if (cv::norm(closest.fieldCoords - balls[target_index].fieldCoords) > 20) { // another ball found
		double t2 = (double)cv::getTickCount();
		double dt = (t2 - ballLost) / cv::getTickFrequency();
		if (dt < 0.5) { // lost too long 
			// predict
			closest.predictCoords();
			*index = -1;
			return closest;
		}
		else {
			reset = true;
		}

	}
	//else{
		closest.filterCoords(balls[target_index], field->self, reset);
		ballLost = (double)cv::getTickCount();
	//}
	return closest;
#endif
}

FieldState::FieldState(int number_of_balls) :yellowGate(YELLOW_GATE), blueGate(BLUE_GATE), self(yellowGate, blueGate, cv::Point(0, 0)), balls(number_of_balls, this){
	gameMode = GAME_MODE_STOPED;
};
FieldState::~FieldState(){

}

void FieldState::resetBallsUpdateState() {
	for (size_t i = 0, isize = balls.size(); i < isize; i++) {
		balls[i].setIsUpdated(false);
	}
}
