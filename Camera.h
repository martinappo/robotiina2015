#pragma  once
#include "types.h"
#include <boost/date_time/posix_time/posix_time.hpp>


class Camera: public ICamera
{
private:
    cv::Mat frame, lastframe, buffer;
	cv::VideoCapture *cap;
	cv::Size frameSize;
	bool flip = false;
	double fps;
	int frames = 0;
	boost::posix_time::time_duration dt;
	boost::posix_time::ptime lastCapture2;
	boost::posix_time::ptime lastCapture;
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

public:
    Camera(const std::string &device);
	Camera(int device);
	Camera();
    const cv::Mat & Capture();
    const cv::Mat & CaptureHSV();
    virtual ~Camera(){ 
		cap->release();
		delete cap;
	}
	virtual cv::Size GetFrameSize(){
		return frameSize;
	};
	virtual double GetFPS() {
		return fps;
	}


};