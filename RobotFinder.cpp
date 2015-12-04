#include "RobotFinder.h"


RobotFinder::RobotFinder()
{
}


RobotFinder::~RobotFinder()
{
}



bool RobotFinder::Locate(cv::Mat &imgThresholded, cv::Mat &frameHSV, cv::Mat &frameBGR, std::vector<cv::Point2i> &objectCoords) {

	try{
		cv::Point2d notValidPosition = cv::Point2d(-1.0, -1.0);
	
		int smallestArea = 20;
		int largestArea = 5000;
		cv::Point2d center(-1, -1);

		if (imgThresholded.rows == 0){
			std::cout << "Image thresholding has failed" << std::endl;
			return false;
		}

		cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

		std::vector<std::vector<cv::Point>> contours; // Vector for storing contour
		std::vector<cv::Vec4i> hierarchy;

		cv::Scalar blackColor(0, 0, 0);
		cv::Scalar whiteColor(255, 255, 255);
		cv::Scalar redColor(0, 0, 255);

		findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image

		if (contours.size() == 0){ //if no contours found
			return false;
		}
		cv::Point2d frameCenter = cv::Point2d(frameHSV.size()) / 2;
	
		for (unsigned int i = 0; i < contours.size(); i++)
		{
			int blobArea = (int)(cv::contourArea(contours[i], false));
			if (blobArea >= smallestArea || blobArea <= largestArea) {
				cv::Moments M = cv::moments(contours[i]);
				cv::Rect bounding_rect = cv::boundingRect(contours[i]);
				rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), redColor, 1, 8, 0);
				try{objectCoords.push_back(cv::Point2d((M.m10 / M.m00), (M.m01 / M.m00)) - frameCenter);}
				catch(cv::Exception ex){return false;}
			}
		}
		return true;
	}catch (cv::Exception ex){ return false; }
}