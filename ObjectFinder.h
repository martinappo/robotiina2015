#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>
#include "KalmanFilter.h"


class ObjectFinder {
protected:
//	virtual cv::Point2i LocateGateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
//	virtual cv::Point2i LocateBallOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
	virtual cv::Point2i LocateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target) { return{ 0, 0 }; };
private:
	void WriteInfoOnScreen(const ObjectPosition &info);
	KalmanFilter* filter = new KalmanFilter(cv::Point2i (400, 400));
	//Vars
	double Hfov = 35.21;
	double Vfov = 21.65; //half of cameras vertical field of view (degrees)
	double CamHeight = 345; //cameras height from ground (mm)
	double CamAngleDev = 26; //deviation from 90* between ground
protected:
	cv::Point2d lastPosition = cv::Point2d(-1.0, -1.0);
public:
	ObjectFinder();
	virtual bool Locate(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, ObjectPosition &targetPos);
	ObjectPosition ConvertPixelToRealWorld(const cv::Point2i &point, const cv::Point2i &frame_size);
	int IsolateField(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, bool detectBothBorders = false, bool nightVision = false);
	virtual void IsolateFieldOld(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR);
	virtual ~ObjectFinder(){ }
	bool LocateCursor(cv::Mat &frameBGR, cv::Point2i cursor, OBJECT target, ObjectPosition &targetPos);

};
