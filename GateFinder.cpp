
#include "GateFinder.h"


GateFinder::GateFinder()
{
}


GateFinder::~GateFinder()
{
}

extern void drawLine(cv::Mat & img, cv::Mat & img2, cv::Vec4f line, int thickness, CvScalar color, bool nightVision = false);

cv::Point2i GateFinder::LocateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target) {
	int smallestGateArea = 1000;
	double growGateHeight = 1.2;
	cv::Point2d center(-1, -1);
	cv::Mat imgThresholded = HSVRanges[target]; // reference counted, I think
	if (imgThresholded.rows == 0) return center;

	cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

	cv::Scalar color(0, 0, 0);
	cv::Scalar color2(255, 255, 255);

	//biggest area calculation
	std::vector<std::vector<cv::Point>> contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;
	cv::Rect bounding_rect;

	findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image
	if (contours.size() == 0){
		return center;
	}

	double largest_area = 0;
	double area = 0;
	size_t largest_contour_index = 0;
	for (size_t i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		area = cv::contourArea(contours[i], false);  //  Find the area of contour		
		if (area > largest_area){
			largest_area = area;
			largest_contour_index = i;                //Store the index of largest contour
		}
	}

	//validate gate area
	if (largest_area < smallestGateArea){
		return cv::Point2i(-1, -1);
	}

	//find center
	if (contours.size() > largest_contour_index){
		cv::Moments M = cv::moments(contours[largest_contour_index]);
		center = cv::Point2i((int)(M.m10 / M.m00), (int)(M.m01 / M.m00));
	}
	else {
		assert(false);
	}

/*
	//Cutting out gate from ball frame	
	bounding_rect = cv::boundingRect(contours[largest_contour_index]);
	bounding_rect.height = bounding_rect.height * growGateHeight;
	rectangle(HSVRanges[BALL], bounding_rect.tl(), bounding_rect.br(), color, -1, 8, 0);
	//for clear visual:
	rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), color, -1, 8, 0);
	if (HSVRanges.find(SIGHT_MASK) != HSVRanges.end()) {
		rectangle(HSVRanges[SIGHT_MASK], bounding_rect.tl(), bounding_rect.br(), color, -1, 8, 0);
	}
*/
	cv::Scalar color4(255, 0, 0);

	cv::RotatedRect bounding_rect2 = cv::minAreaRect(contours[largest_contour_index]);
	cv::Point2f rect_points[4]; bounding_rect2.points(rect_points);
	int min_dist = INT_MAX;
	int min_index = 0;
	int min_dist2 = INT_MAX;
	int min_index2 = 0;
	for (int i = 0; i < 4; i++){
		int dist = cv::norm(rect_points[i] - cv::Point2f(imgThresholded.cols / 2, imgThresholded.rows / 2));
		if (dist < min_dist){
			min_dist2 = min_dist;
			min_index2 = min_index;
			min_dist = dist;
			min_index = i;
		}
		else if (dist < min_dist2){
			min_dist2 = dist;
			min_index2 = i;
		}
	}
	center = (rect_points[min_index]+ rect_points[min_index2])/2;	
	circle(frameBGR,  center, 7, color, -1, 8, 0);

	
	int shift = bounding_rect2.size.height * 0.09 +0.2;
	//std::cout << "shift: " << shift << " height: " << bounding_rect2.size.height << std::endl;
	for (int j = 0; j < 4; j++) {
		line(frameBGR, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
		/*
		std::vector<cv::Point2i> points;
		points.push_back(rect_points[j]);
		points.push_back(rect_points[(j + 1) % 4]);
		cv::Vec4f newLine;
		cv::fitLine(points, newLine, CV_DIST_L2, 0, 0.1, 0.1);
		//std::cout << rect_points[j] << ", " << rect_points[(j + 1) % 4] << ": " << atan2(newLine[1], newLine[0])*180/PI << std::endl;
		if (abs(atan2(newLine[1], newLine[0]) * 180 / PI) < 45) {
			newLine[3] += shift; // shift line down
			drawLine(frameBGR, HSVRanges[BALL], newLine, 1, cv::Scalar(0, 255 * (1 + 0.3), 0));
			drawLine(frameBGR, HSVRanges[SIGHT_MASK], newLine, 1, cv::Scalar(0, 255 * (1 + 0.3), 0));
		}
		*/

	}
	

	// find out lowest line
	return center;
}
