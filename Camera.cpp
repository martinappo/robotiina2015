#include "Camera.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#define DOUBLE_BUFFERING

Camera::Camera(const std::string &device) {
	cap = new cv::VideoCapture(device.c_str());
	if (!cap->isOpened())  // if not success, exit program
	{
		throw std::runtime_error("Camera not found");
	}
	Init();
}
void Camera::Init() {
	frameCount = (int)(cap->get(CV_CAP_PROP_FRAME_COUNT));

	*cap >> frame;
	frameSize = cv::Size(frame.size());

	frame.copyTo(frame1);
	frame.copyTo(frame2);

	//auto _roi = roi;

	if (frame.cols < roi.height || frame.rows < roi.width) {
		auto _roi = roi;
		roi = cv::Rect(0, 0, frameSize.width, frameSize.height);
		std::cout << "Camera ROI [" << _roi << "] is bigger than frame size [" << frameSize << "], using full frame" << std::endl;
	}
	frame1_roi = frame1(roi);
	frame2_roi = frame2(roi);


//	frameSize = cv::Size((int)cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
//		(int)cap->get(CV_CAP_PROP_FRAME_HEIGHT));

	maskedImage = cv::Mat(roi.height, roi.width, CV_8UC3, cv::Scalar(255, 0, 255));
	mask = cv::Mat(roi.height, roi.width, CV_8U, cv::Scalar::all(0));
	int radius = roi.height / 2;
	cv::circle(mask, cv::Point(radius, radius), radius, cv::Scalar(255, 255, 255), -1, 8, 0);


	if (frameCount == 1) { // image
		return;
	}

	/*
	cap->set(CV_CAP_PROP_FPS, 60);
	cap->set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap->set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	*/
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
	Init();

}


cv::Mat &Camera::GetLastFrame(bool bFullFrame){
	return *m_pFrame;
}

cv::Mat &Camera::Capture(bool bFullFrame){

	if (frameCount == 1) { // image
		frame.copyTo(frame1); // return clean copy
		//return frame1_roi;
	}
	else {
#ifndef DOUBLE_BUFFERING
		if (cap->isOpened()){
			*cap >> *m_pFrame;
		}
#else
		if (bCaptureNextFrame) {
			//	std::cout << "Requesting too fast, next frame not ready!" << std::endl;
			while (bCaptureNextFrame && !stop_thread){
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
#endif
	}
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
	
		m_pFrame->copyTo(maskedImage, mask);

		return maskedImage;
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
		cv::Mat &nextFrame = bCaptureFrame1 ? frame1 : frame2;
		cv::Mat &nextRoi = bCaptureFrame1 ? frame1_roi : frame2_roi;

		if (cap->isOpened()) {
			*cap >> nextFrame;
		}
		else {
			bCaptureNextFrame = false;
		}
		if (cap->get(CV_CAP_PROP_POS_FRAMES) >= frameCount){ //restartVideo
			cap->set(CV_CAP_PROP_POS_FRAMES, 0);
			continue;

		}
		if (nextFrame.size().height == 0) {
			cap->set(CV_CAP_PROP_POS_FRAMES, 0);
			bCaptureNextFrame = true;
			std::cout << "Invalid frame captured " << frame1.size() << std::endl;
			continue;
		}


		//maskimine koos pildi väiksemaks lõikamisega
		
		//frame_roi = nextFrame(cv::Rect(175, 60, frameSize.width, frameSize.height));
		m_pFrame = &nextRoi; // without mask
		bCaptureFrame1 = !bCaptureFrame1;
		bCaptureNextFrame = false;

	}
}