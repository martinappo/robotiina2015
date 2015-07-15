#include "camera.h"
#include <opencv2/opencv.hpp>

Camera::Camera(const std::string &device)
{

	cap = new cv::VideoCapture(device.c_str());
	if (!cap->isOpened())  // if not success, exit program
    {
		throw std::runtime_error("Camera not found");
    }
	//cap->set(CV_CAP_PROP_FPS, 60);
	//cap->set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	//cap->set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	/*
	https://github.com/jaantti/Firestarter/blob/master/2014/run.sh
	cap->set(CV_CAP_PROP_EXPOSURE, -5);
	cap->set(CV_CAP_PROP_BRIGHTNESS, 0);
	cap->set(CV_CAP_PROP_HUE, 0);
	cap->set(CV_CAP_PROP_SATURATION, 80);
	cap->set(CV_CAP_PROP_CONTRAST, 5);
	cap->set(CV_CAP_PROP_RECTIFICATION, 1);*/

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
	/*
	cap->set(CV_CAP_PROP_EXPOSURE, -5);
	cap->set(CV_CAP_PROP_BRIGHTNESS, 0);
	cap->set(CV_CAP_PROP_HUE, 0);
	cap->set(CV_CAP_PROP_SATURATION, 80);
	cap->set(CV_CAP_PROP_CONTRAST, 5);
	cap->set(CV_CAP_PROP_RECTIFICATION, 1);*/

}

const cv::Mat &Camera::Capture()
{
	if (cap->isOpened())
		
		*cap >> frame;
		
	if (frame.size().height > 0) {
		frame.copyTo(lastframe);
	}
	if (flip)
		cv::flip(lastframe, buffer, -1);
	else
		lastframe.copyTo(buffer);

	return buffer;
}
const cv::Mat &Camera::CaptureHSV() {
    cvtColor(frame, buffer, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    return buffer;
}
