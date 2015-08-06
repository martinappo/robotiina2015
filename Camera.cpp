#include "Camera.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#define DOUBLE_BUFFERING

Camera::Camera(const std::string &device){

	cap = new cv::VideoCapture(device.c_str());
	if (!cap->isOpened())  // if not success, exit program
    {
		throw std::runtime_error("Camera not found");
    }

	frameSize = cv::Size((int)cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int)cap->get(CV_CAP_PROP_FRAME_HEIGHT));

	cap->set(CV_CAP_PROP_FPS, 60);
	cap->set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap->set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	/*
	https://github.com/jaantti/Firestarter/blob/master/2014/run.sh
	cap->set(CV_CAP_PROP_EXPOSURE, -5);
	cap->set(CV_CAP_PROP_BRIGHTNESS, 0);
	cap->set(CV_CAP_PROP_HUE, 0);
	cap->set(CV_CAP_PROP_SATURATION, 80);
	cap->set(CV_CAP_PROP_CONTRAST, 5);
	cap->set(CV_CAP_PROP_RECTIFICATION, 1);*/
#ifdef DOUBLE_BUFFERING
	Start();
#endif
}

Camera::Camera(int device)
{	
	cap = new cv::VideoCapture(device);
	if (!cap->isOpened())  // if not success, exit program
	{
		throw std::runtime_error("Camera is missing");
	}
	frameSize = cv::Size((int)cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int)cap->get(CV_CAP_PROP_FRAME_HEIGHT));
	flip = false;

	cap->set(CV_CAP_PROP_FPS, 60);
	cap->set(CV_CAP_PROP_FRAME_WIDTH, 800);
	cap->set(CV_CAP_PROP_FRAME_HEIGHT, 600);
	/*
	cap->set(CV_CAP_PROP_EXPOSURE, -5);
	cap->set(CV_CAP_PROP_BRIGHTNESS, 0);
	cap->set(CV_CAP_PROP_HUE, 0);
	cap->set(CV_CAP_PROP_SATURATION, 80);
	cap->set(CV_CAP_PROP_CONTRAST, 5);
	cap->set(CV_CAP_PROP_RECTIFICATION, 1);*/
#ifdef DOUBLE_BUFFERING
	Start();
#endif

}

const cv::Mat &Camera::Capture(){
#ifndef DOUBLE_BUFFERING
	if (cap->isOpened()){
		*cap >> frame;
	}
#endif
	/*
	if (frame.size().height > 0) {
		frame.copyTo(lastframe);
	}
	if (flip)
		cv::flip(lastframe, buffer, -1);
	else
		lastframe.copyTo(buffer);
    */
	//time = boost::posix_time::microsec_clock::local_time();
	//boost::posix_time::time_duration::tick_type dt = (time - lastCapture).total_milliseconds();
	boost::posix_time::time_duration::tick_type dt2 = (time - lastCapture2).total_milliseconds();
/*
	if (dt < 24){
		std::this_thread::sleep_for(std::chrono::milliseconds(12)); // limit fps to about 50fps
	}
*/
	if (frames > 10) {
		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type dt2 = (time - lastCapture2).total_milliseconds();
		fps = 1000.0 * frames / dt2;
		lastCapture2 = time;
		frames = 0;
	}
	else {
		frames++;
	}
	bCaptureNextFrame = true;
#ifdef DOUBLE_BUFFERING
	return *m_pFrame;
#else
	//lastCapture = time;
	return frame;
#endif
}
const cv::Mat &Camera::CaptureHSV() {
    cvtColor(frame, buffer, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    return buffer;
}

void Camera::Run(){
	while (!stop_thread) {
		if (!bCaptureNextFrame) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		bCaptureNextFrame = false;
		if (bCaptureFrame1) {
			*cap >> frame1;
			m_pFrame = &frame1;
			bCaptureFrame1 = !bCaptureFrame1;
		}
		else {
			*cap >> frame2;
			m_pFrame = &frame2;
			bCaptureFrame1 = !bCaptureFrame1;

		}


	}
}