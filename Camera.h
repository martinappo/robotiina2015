#pragma  once
#include "types.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "ThreadedClass.h"

class Camera: public ICamera, public ThreadedClass
{
	enum {
		CAPTURE_FRAME1 = 0,
		RETURN_FRAME1,
		CAPTURE_FRAME2,
		RETURN_FRAME2
	};
private:
    cv::Mat frame, frame1, frame2, lastframe, buffer;
	std::atomic_bool bCaptureNextFrame;
	std::atomic_bool bCaptureFrame1;
	cv::Mat* m_pFrame = &frame1;
	cv::VideoCapture *cap;
	cv::Size frameSize;
	bool flip = false;
	double fps;
	int frames = 0;
	boost::posix_time::time_duration dt;
	boost::posix_time::ptime lastCapture2;
	boost::posix_time::ptime lastCapture;
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

protected:
	void Run();
	void Start() {
		bCaptureFrame1 = true;
		bCaptureNextFrame = true;
		ThreadedClass::Start();
	}
public:
    Camera(const std::string &device);
	Camera(int device);
	Camera();
	cv::Mat & Capture();
    const cv::Mat & CaptureHSV();
    virtual ~Camera(){ 
		WaitForStop();
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
