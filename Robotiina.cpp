// Robotiina.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "Robot.h"
#include "RemoteControl.h"



int main(int argc, char *argv[])
{
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

	cv::VideoCapture *cap = new cv::VideoCapture(CV_CAP_XIAPI);
	cv::Size frameSize = cv::Size((int)cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int)cap->get(CV_CAP_PROP_FRAME_HEIGHT));
	cv::Mat frame;
	double fps = 0;
	boost::posix_time::time_duration::tick_type dt2;
	double frames = 0;

	boost::posix_time::time_duration dt;
	boost::posix_time::ptime lastCapture2;
	boost::posix_time::ptime lastCapture;

	cv::namedWindow("Display window", CV_WINDOW_AUTOSIZE);// Create a window for display.
	while (1){
		*cap >> frame;
		cv::imshow("Display window", frame);
		cv::waitKey(1);


		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type dt = (time - lastCapture).total_milliseconds();
		boost::posix_time::time_duration::tick_type dt2 = (time - lastCapture2).total_milliseconds();
		if (dt2 > 10000) {
			fps = 1000.0 * frames / dt2;
			lastCapture2 = time;
			frames = 0;
			std::cout << fps;
			std::cout << "\n";
		}
		else {
			frames++;
		}

	}
//
//
//	if (!cap->isOpened())  // if not success, exit program
//	{
//		throw std::runtime_error("Camera is missing");
//	}
//	frameSize = cv::Size((int)cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
//		(int)cap->get(CV_CAP_PROP_FRAME_HEIGHT));
//
//
//    boost::asio::io_service io;
//    Robot robotiina(io);
////    RemoteControl sr(io, &robotiina);
//
//	try
//	{
////           sr.Start();
//
//		robotiina.Launch(argc, argv);
//    }
//	catch (std::exception &e)
//	{
//		std::cout << "ups, " << e.what() << std::endl;
//		throw;
//	}
//	catch (const std::string &e)
//	{
//		std::cout << "ups, " << e << std::endl;
//	}
//	catch (...)
//	{
//		std::cout << "ups, did not see that coming."<< std::endl;
//	}
////    sr.Stop();
    return 0;

}
