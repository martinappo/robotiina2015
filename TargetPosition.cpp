#include "TargetPosition.h"


TargetPosition::~TargetPosition()
{
}


void TargetPosition::updateFieldCoords(cv::Point orgin) {

}

TargetPosition::TargetPosition(cv::Point orgin){
	fieldCoords = orgin;
}

TargetPosition::TargetPosition(BallPosition orgin){
	fieldCoords = orgin.fieldCoords; 
	rawPixelCoords = orgin.rawPixelCoords;;
	polarMetricCoords = orgin.polarMetricCoords;
}
