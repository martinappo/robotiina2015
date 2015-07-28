#pragma  once
#include "types.h"

class Camera: public ICamera
{
private:
    cv::Mat frame, lastframe, buffer;
	cv::VideoCapture *cap;
	cv::Size frameSize;
	bool flip = false;
public:
    Camera(const std::string &device);
	Camera(int device);
    const cv::Mat & Capture();
    const cv::Mat & CaptureHSV();
    virtual ~Camera(){ 
		cap->release();
		delete cap;
	}
	virtual cv::Size GetFrameSize(){
		return frameSize;
	};

};