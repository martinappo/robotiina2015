#include "StillCamera.h"

StillCamera::StillCamera(const std::string &fileName)
{
	_frame = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);   // Read the file
}
const cv::Mat &StillCamera::Capture()
{
	_frame.copyTo(frame);
	return frame;
}