#include "Camera.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>

Camera::Camera(const std::string &device){

	cap = new cv::VideoCapture(device.c_str());
	if (!cap->isOpened())  // if not success, exit program
    {
		throw std::runtime_error("Camera not found");
    }

	frameSize = cv::Size((int)cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int)cap->get(CV_CAP_PROP_FRAME_HEIGHT));

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
	image.size = sizeof(XI_IMG);
	image.bp = NULL;
	image.bp_size = 0;
	DWORD dwNumberOfDevices = 0;
	stat = xiGetNumberDevices(&dwNumberOfDevices);
	stat = xiOpenDevice(0, &handle);
	char serial_number1[100] = "";
	xiGetParamString(handle, XI_PRM_DEVICE_SN, serial_number1, sizeof(serial_number1));
	stat = xiSetParamInt(handle, XI_PRM_TRG_SOURCE, XI_TRG_SOFTWARE);
	stat = xiSetParamInt(handle, XI_PRM_GPO_SELECTOR, 2);
	stat = xiSetParamInt(handle, XI_PRM_GPO_MODE, XI_GPO_FRAME_ACTIVE_NEG);
	stat = xiSetParamInt(handle, XI_PRM_EXPOSURE, 2000);
	stat = xiSetParamInt(handle, XI_PRM_GAIN, 5);
	stat = xiStartAcquisition(handle);
	int ival;
	xiGetParamInt(handle, XI_PRM_EXPOSURE, &ival);
	int isColor = 0;
	stat = xiGetParamInt(handle, XI_PRM_IMAGE_IS_COLOR, &isColor);
	if (isColor) {
		stat = xiSetParamInt(handle, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB24);
		stat = xiSetParamInt(handle, XI_PRM_AUTO_WB, 1);
	}
	stat = xiSetParamInt(handle, XI_PRM_DOWNSAMPLING_TYPE, XI_SKIPPING);
	stat = xiSetParamInt(handle, XI_PRM_DOWNSAMPLING, 4);
	int w;
	xiGetParamInt(handle, XI_PRM_WIDTH, &w);
	int h;
	xiGetParamInt(handle, XI_PRM_HEIGHT, &h);
	stat = xiSetParamInt(handle, XI_PRM_WIDTH, w);
	stat = xiSetParamInt(handle, XI_PRM_HEIGHT, h);
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

Camera::Camera(){
	image.size = sizeof(XI_IMG);
	image.bp = NULL;
	image.bp_size = 0;
	DWORD dwNumberOfDevices = 0;
	stat = xiGetNumberDevices(&dwNumberOfDevices);
	stat = xiOpenDevice(0, &handle);
	char serial_number1[100] = "";
	xiGetParamString(handle, XI_PRM_DEVICE_SN, serial_number1, sizeof(serial_number1));
	stat = xiSetParamInt(handle, XI_PRM_TRG_SOURCE, XI_TRG_SOFTWARE);
	stat = xiSetParamInt(handle, XI_PRM_GPO_SELECTOR, 2);
	stat = xiSetParamInt(handle, XI_PRM_GPO_MODE, XI_GPO_FRAME_ACTIVE_NEG);
	stat = xiSetParamInt(handle, XI_PRM_EXPOSURE, 2000);
	stat = xiSetParamInt(handle, XI_PRM_GAIN, 5);
	stat = xiStartAcquisition(handle);
	int ival;
	xiGetParamInt(handle, XI_PRM_EXPOSURE, &ival);
	int isColor = 0;
	stat = xiGetParamInt(handle, XI_PRM_IMAGE_IS_COLOR, &isColor);
	if (isColor) {
		stat = xiSetParamInt(handle, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB24);
		stat = xiSetParamInt(handle, XI_PRM_AUTO_WB, 1);
	}
	stat = xiSetParamInt(handle, XI_PRM_DOWNSAMPLING_TYPE, XI_SKIPPING);
	stat = xiSetParamInt(handle, XI_PRM_DOWNSAMPLING, 4);
	int w;
	xiGetParamInt(handle, XI_PRM_WIDTH, &w);
	int h;
	xiGetParamInt(handle, XI_PRM_HEIGHT, &h);
	stat = xiSetParamInt(handle, XI_PRM_WIDTH, w);
	stat = xiSetParamInt(handle, XI_PRM_HEIGHT, h);
	usingXimea = true;
}

const cv::Mat &Camera::Capture(){

	if (usingXimea){
		xiSetParamInt(handle, XI_PRM_TRG_SOFTWARE, 0);
		stat = xiGetImage(handle, 5000, &image);
		cv::Mat tFrame;
		if (stat != XI_OK)
			printf("Error after xiGetimage (%d)\n", stat);
		else{
			cv::Mat frame1(image.height, image.width, CV_8UC3, image.bp);
			frame1.copyTo(frame);
		}
	}else{
		if (cap->isOpened()){
			*cap >> frame;
		}
	}
	if (frame.size().height > 0) {
		frame.copyTo(lastframe);
	}
	if (flip)
		cv::flip(lastframe, buffer, -1);
	else
		lastframe.copyTo(buffer);

	time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type dt = (time - lastCapture).total_milliseconds();
	boost::posix_time::time_duration::tick_type dt2 = (time - lastCapture2).total_milliseconds();
	if (dt < 24){
		std::this_thread::sleep_for(std::chrono::milliseconds(12)); // limit fps to about 50fps
	}

	if (dt2 > 1000) {
		fps = 1000.0 * frames / dt2;
		lastCapture2 = time;
		frames = 0;
	}
	else {
		frames++;
	}
	lastCapture = time;
	return buffer;
}
const cv::Mat &Camera::CaptureHSV() {
    cvtColor(frame, buffer, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    return buffer;
}
