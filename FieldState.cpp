#include "FieldState.h"

void FieldState::resetBallsUpdateState() {
	for (int i = 0; i < NUMBER_OF_BALLS; i++) {
		balls[i].setIsUpdated(false);
	}
}
