#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>
#include "FieldState.h"


class ObjectFinder {
private:
	//Vars
	double Hfov = 35.21;
	double Vfov = 21.65; //half of cameras vertical field of view (degrees)
	double CamHeight = 345; //cameras height from ground (mm)
	double CamAngleDev = 26; //deviation from 90* between ground
protected:
	cv::Point2d lastPosition = cv::Point2d(-1.0, -1.0);
public:
	ObjectFinder();
	virtual bool Locate(cv::Mat &threshHoldedImage, cv::Mat &frameHSV, cv::Mat &frameBGR, ObjectPosition & objectPos) = 0;
	virtual ~ObjectFinder(){ }
};
