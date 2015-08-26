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

	frameCount = cap->get(CV_CAP_PROP_FRAME_COUNT);
	m_pFrame = &frame1;

	if (frameCount == 1) { // image
		*cap >> frame;
		frameSize = cv::Size(frame.size());
		return;
	}

	//v�iksemaks l�igatud frame'i suurus
//	frameSize = cv::Size(960,    // Acquire input size
//		960);

	frameSize = cv::Size((int)cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int)cap->get(CV_CAP_PROP_FRAME_HEIGHT));

	std::cout << frameSize.width << ", " << frameSize.height << std::endl;




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
	frameSize = cv::Size((int)cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int)cap->get(CV_CAP_PROP_FRAME_HEIGHT));
	flip = false;
	frameCount = cap->get(CV_CAP_PROP_FRAME_COUNT);

	cap->set(CV_CAP_PROP_FPS, 60);
	//cap->set(CV_CAP_PROP_FRAME_WIDTH, 800);
	//cap->set(CV_CAP_PROP_FRAME_HEIGHT, 600);
	/*
	cap->set(CV_CAP_PROP_EXPOSURE, -5);
	cap->set(CV_CAP_PROP_BRIGHTNESS, 0);
	cap->set(CV_CAP_PROP_HUE, 0);
	cap->set(CV_CAP_PROP_SATURATION, 80);
	cap->set(CV_CAP_PROP_CONTRAST, 5);
	cap->set(CV_CAP_PROP_RECTIFICATION, 1);*/
	// blank frame to show before capture starts
	frame = cv::Mat(frameSize, CV_8UC3);
	m_pFrame = &frame;
#ifdef DOUBLE_BUFFERING
	Start();
#endif

}

cv::Mat &Camera::Capture(){
	if (frameCount == 1) { // image
		frame.copyTo(*m_pFrame); // return clean copy  
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
	
	return *m_pFrame;
}
const cv::Mat &Camera::CaptureHSV() {
    cvtColor(frame, buffer, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    return buffer;
}

void Camera::Run(){
	frameCounter = 0;
	while (!stop_thread) {
		if (!bCaptureNextFrame) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		cv::Mat &nextFrame = bCaptureFrame1 ? frame1 : frame2;

		if (cap->isOpened()) {
			*cap >> nextFrame;
			frameCounter++;
		}
		else {
			bCaptureNextFrame = false;
		}
		if (frameCounter == frameCount){ //restartVideo
			frameCounter = 0;
			cap->set(CV_CAP_PROP_POS_FRAMES, 0);
			continue;

		}
		if (nextFrame.size().height == 0) {
			cap->set(CV_CAP_PROP_POS_FRAMES, 0);
			bCaptureNextFrame = true;
			std::cout << "Invalid frame captured " << frame1.size() << std::endl;
			continue;
		}
		
		//maskimine koos pildi v�iksemaks l�ikamisega
		/*
		frame_roi = nextFrame(cv::Rect(175, 60, frameSize.width, frameSize.height));
		std::cout << frame_roi.rows << ", " << frame_roi.cols << std::endl;
		cv::Mat maskedImage;
		cv::Mat mask(frame_roi.rows, frame_roi.cols, CV_8U, cv::Scalar::all(0));
		mask.setTo(cv::Scalar(0, 0, 0));
		int radius = frame_roi.rows / 2;
		cv::circle(mask, cv::Point(radius, radius), radius, cv::Scalar(255, 255, 255), -1, 8, 0);
		frame_roi.copyTo(maskedImage, mask);
		cv::imshow("test pilt", maskedImage);
		cv::waitKey(1);
		*/

		//maskimine
		cv::Mat maskedImage(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(255,0,255));
		cv::Mat mask(frameSize.height, frameSize.width, CV_8U, cv::Scalar::all(0));

		cv::circle(mask, cv::Point((frameSize.width / 2) + 10, (frameSize.height / 2) + 20), 480, cv::Scalar(255, 255, 255), -1, 8, 0);
		nextFrame.copyTo(maskedImage, mask);
//		cv::imshow("test pilt", maskedImage);
//		cv::waitKey(1);

		m_pFrame = &nextFrame;
		bCaptureFrame1 = !bCaptureFrame1;
		bCaptureNextFrame = false;

	}
}