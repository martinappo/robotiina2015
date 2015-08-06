#include "BallFinder.h"


BallFinder::BallFinder()
{
}


BallFinder::~BallFinder()
{
}


cv::Point2i BallFinder::LocateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target) {
	cv::Point2d notValidPosition = cv::Point2d(-1.0, -1.0);
	ballCountLeft=0;
	ballCountRight=0;
	
	int smallestBallArea = 4;
	cv::Point2d center(-1, -1);
	cv::Mat imgThresholded = HSVRanges[target]; // reference counted, I think
	cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

	std::vector<std::vector<cv::Point>> contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;

	cv::Scalar color(0, 0, 0);
	cv::Scalar color2(255, 255, 255);
	cv::Scalar color3(0, 0, 255);

	findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image

	if (contours.size() == 0){ //if no contours found
		return center;
	}
	//the geater the closest
	double ball_distance = 0;
	double ball_shift = 0;
	double closest_distance = 0;
	std::vector<std::pair<int, int> > ball_indexes;
	for (int i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		int area = cv::contourArea(contours[i], false);
		if (area < smallestBallArea){ //validate ball's size
			ball_distance = 0;
		}
		else{
			cv::Moments M = cv::moments(contours[i]);
			ball_distance = M.m01 / M.m00;
			ball_shift = M.m10 / M.m00;
			ball_indexes.push_back(std::make_pair(ball_distance, i));
			//if (ball_distance > 200) {
				if (ball_shift < 320) ballCountLeft++;
				if (ball_shift > 320) ballCountRight++;			
			//}
			
	cv::Rect bounding_rect = cv::boundingRect(contours[i]);
	rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), color3, 1, 8, 0);
			
		}
	}

	if (ball_indexes.empty()){
		return center;
	}

	int closest_ball_index = 0;
	cv::Point2d closestBall = cv::Point2d(-1, -1);
	std::sort(ball_indexes.begin(), ball_indexes.end());
	while (!ball_indexes.empty()){
		//If there is nothing to compare with
		closest_ball_index = ball_indexes.back().second;
		//Choosing new ball
		if (lastPosition.x < 0 && lastPosition.y < 0){
			if (contours.size() > closest_ball_index){
				cv::Moments M = cv::moments(contours[closest_ball_index]);
				closestBall = cv::Point2d(M.m10 / M.m00, M.m01 / M.m00);
			}
			else {
				assert(false);
			}
			//If we found not valid ball
			if (cv::norm(closestBall - notValidPosition) < 100 && notValidPosition.x > 0 && notValidPosition.y > 0){
				return cv::Point2i(-2, -2);
			}
			else{
				notValidPosition = cv::Point2i(-1, -1); //reset variable
			}
		}
		//Comparing with prev result
		else{
			double smallestDistance = 9999;

			for (int i = 0; i < ball_indexes.size(); i++){
				cv::Moments M = cv::moments(contours[ball_indexes[i].second]);
				center = cv::Point2d(M.m10 / M.m00, M.m01 / M.m00);
				double distance = cv::norm(center - lastPosition);
				if (smallestDistance > distance){
					smallestDistance = distance;
					closestBall = center;
				}
			}
			//distance between found ball and chosen ball
			if (smallestDistance > 609){
				return cv::Point2d(-1, -1);
			}

		}
		/*
		//VALIDATE BALL
		//For ball validation, drawed contour should cover balls shadow.
		int thickness = (int)ceil(cv::contourArea(contours[closest_ball_index], false) / 50);
		thickness = std::min(100, std::max(thickness, 12));
		drawContours(HSVRanges[OUTER_BORDER], contours, closest_ball_index, color, thickness, 8, hierarchy);
		drawContours(HSVRanges[OUTER_BORDER], contours, closest_ball_index, color, -5, 8, hierarchy);
		drawContours(frameBGR, contours, closest_ball_index, color, thickness, 8, hierarchy);
		drawContours(frameBGR, contours, closest_ball_index, color, -5, 8, hierarchy);
		*/
		bool valid = validateBall(HSVRanges, closestBall, frameHSV, frameBGR);
		if (!valid){
			notValidPosition = closestBall;
			cv::circle(frameBGR, closestBall, 5, cv::Scalar(100, 0, 225), 3); //not valid ball is purple
		}
		else{
			cv::circle(frameBGR, closestBall, 12, cv::Scalar(225, 225, 225), 2); //valid ball is white
			return	closestBall;
		}
		ball_indexes.pop_back();
	}
	//If there is not valid ball
	return cv::Point2i(-2, -2);

}

bool BallFinder::validateBall(ThresholdedImages &HSVRanges, cv::Point2d endPoint, cv::Mat &frameHSV, cv::Mat &frameBGR)
{
	return true;
	cv::Mat innerThresholded = HSVRanges[INNER_BORDER];
	cv::Mat outerThresholded = HSVRanges[OUTER_BORDER];
	cv::Mat fieldThresholded = HSVRanges[FIELD];

	cv::Point startPoint;
	startPoint.x = frameHSV.cols / 2;
	startPoint.y = frameHSV.rows;

	cv::LineIterator iterator(frameHSV, startPoint, endPoint, 8);
	int behindLineCount = 0;
	int alterIterator = -10;
	for (int n = 0; n < 10; n++){
		cv::Point2d lastInner = { 0, 0 };
		cv::Point2d firstInner = { 0, 0 };
		cv::Point2d lastOuter = { 0, 0 };
		cv::Point2d firstOuter = { 0, 0 };

		std::string state = "inner";
		bool Hinrange = false;
		bool Sinrange = false;
		bool Vinrange = false;
		bool firstFound = false;
		bool fieldFound = false;
		iterator = cv::LineIterator{ frameHSV, cv::Point(startPoint.x + alterIterator, startPoint.y), cv::Point(endPoint.x + alterIterator, endPoint.y), 8 };
		for (int i = 0; i < iterator.count; i++, ++iterator)
		{
			if (state == "inner"){
				bool inRange = innerThresholded.ptr<uchar>(iterator.pos().y)[iterator.pos().x] == 255;
				if (inRange){
					firstInner = iterator.pos();
					state = "outer";
				}
			}
			else if (state == "outer"){
				bool inRange = outerThresholded.ptr<uchar>(iterator.pos().y)[iterator.pos().x] == 255;
				bool fieldInRange = fieldThresholded.ptr<uchar>(iterator.pos().y)[iterator.pos().x] == 255;
				if (fieldInRange && !fieldFound){
					if (!firstFound){
						fieldFound = true; //Found field between inner and outer
					}
				}
				else if (inRange && !firstFound){
					firstFound = true;
					firstOuter = iterator.pos();
				}
				else if (inRange){
					lastOuter = iterator.pos();
				}
				inRange = innerThresholded.ptr<uchar>(iterator.pos().y)[iterator.pos().x] == 255;
				if (inRange){
					lastInner = iterator.pos();
				}

			}
		}//lineiterator end
		alterIterator = alterIterator + 2;
		cv::circle(frameBGR, firstOuter, 5, cv::Scalar(0, 0, 0), -1);
		cv::circle(frameBGR, lastInner, 5, cv::Scalar(200, 200, 200), -1);

		double distLiFo = cv::norm(lastInner - firstOuter);

		if (!firstFound || fieldFound){
			continue;
		}
		if (distLiFo < 20){
			behindLineCount++;
		}
	}//ten times end

	return !(behindLineCount >= 4);
}
