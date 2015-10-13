#include "GatePosition.h"


GatePosition::GatePosition(OBJECT gate) {
	if (gate == YELLOW_GATE) {
		this->polarMetricCoords = cv::Point(0, 230);
		this->fieldCoords = cv::Point(0, 230);
	}
	else if (gate == BLUE_GATE) {
		this->polarMetricCoords = cv::Point(0, -230);
		this->fieldCoords = cv::Point(0, -230);
	}
	else {
		throw std::runtime_error("Gate must be either BLUE_GATE or YELLOW_GATE");
	}
}

GatePosition::~GatePosition()
{
}

