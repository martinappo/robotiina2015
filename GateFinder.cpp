
#include "GateFinder.h"


GateFinder::GateFinder()
{
}


GateFinder::~GateFinder()
{
}

extern void drawLine(cv::Mat & img, cv::Mat & img2, cv::Vec4f line, int thickness, CvScalar color, bool nightVision = false);

bool GateFinder::Locate(cv::Mat &imgThresholded, cv::Mat &frameHSV, cv::Mat &frameBGR, cv::Point &center, cv::Point2f *bounds, std::vector<cv::Point2i> &notGates) {
	int smallestGateArea = 1;
	double growGateHeight = 1.2;
	center = cv::Point(-1, -1);
	//cv::Mat imgThresholded = HSVRanges[target]; // reference counted, I think
	if (imgThresholded.rows == 0) return false;

	cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

	cv::Scalar color(0, 0, 0);
	cv::Scalar color2(255, 255, 255);

	//biggest area calculation
	std::vector<std::vector<cv::Point>> contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;

	findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image
	if (contours.size() == 0){
		return false;
	}

	double largest_area = 0;
	double area = 0;
	size_t sec_largest_contour_index = 0;
	size_t largest_contour_index = 0;

	for (size_t i = 0; i < contours.size(); i++)
	{
		area = cv::contourArea(contours[i], false);
		if (area > largest_area){
			notGates.push_back(getCenterFromContour(contours[i]));
			largest_area = area;
			sec_largest_contour_index = largest_contour_index;
			largest_contour_index = i;  //Store the index of largest contour
		}
	}

	//validate gate area
	if (largest_area < smallestGateArea){
		return false;
	}


	

	if (contours.size() <= largest_contour_index) {
		assert(false);
	}

	std::vector<cv::Point> merged_contour_points;
	//merge two contours if they are close	
	if (contours.size() > sec_largest_contour_index &&
		(cv::norm(getCenterFromContour(contours[largest_contour_index]) - 
			getCenterFromContour(contours[sec_largest_contour_index])) < 500)) 
	{
		for (int i = 0; i < contours[largest_contour_index].size(); i++) {
			merged_contour_points.push_back(contours[largest_contour_index][i]);
		}
		for (int j = 0; j < contours[sec_largest_contour_index].size(); j++) {
			merged_contour_points.push_back(contours[sec_largest_contour_index][j]);
		}

	}
	else {
		merged_contour_points = contours[largest_contour_index];
	}

	if (merged_contour_points.size() > 0) {
		center = getCenterFromContour(merged_contour_points);
		cv::RotatedRect bounding_rect2 = cv::minAreaRect(merged_contour_points);
		bounding_rect2.points(bounds);

		for (int j = 0; j < 4; j++) {
			line(frameBGR, bounds[j], bounds[(j + 1) % 4], color, 1, 8);
		}
	}
	else {
		return false;
	}

	return true;
}

cv::Point2i GateFinder::getCenterFromContour(std::vector<cv::Point> contour) {
	cv::Moments M = cv::moments(contour);
	return cv::Point2i((int)(M.m10 / M.m00), (int)(M.m01 / M.m00));
}
