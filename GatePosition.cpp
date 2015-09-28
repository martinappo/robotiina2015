#include "GatePosition.h"


GatePosition::GatePosition(OBJECT gate) {
	if (gate == YELLOW_GATE) {
		this->polarMetricCoords = cv::Point(0, 225);
		this->fieldCoords = cv::Point(0, 225);
	}
	else if (gate == BLUE_GATE) {
		this->polarMetricCoords = cv::Point(0, -225);
		this->fieldCoords = cv::Point(0, -225);
	}
	else {
		throw std::runtime_error("Gate must be either BLUE_GATE or YELLOW_GATE");
	}
}

GatePosition::~GatePosition()
{
}

