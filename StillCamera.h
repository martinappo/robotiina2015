#pragma  once
#include "types.h"
/*
Because Camera is not working on windows with single image (next frame is invalid)
*/
class StillCamera : public ICamera, protected cv::VideoCapture
{
private:
	cv::Mat _frame;
	cv::Mat frame;
public:
	StillCamera(const std::string &fileName);
	cv::Mat & Capture();
	virtual ~StillCamera(){ }

};