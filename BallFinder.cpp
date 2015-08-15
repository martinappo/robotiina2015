#include "BallFinder.h"


BallFinder::BallFinder()
{
}


BallFinder::~BallFinder()
{
}


void BallFinder::PopulateBalls(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, FieldState &fieldState) {
	cv::Point2d notValidPosition = cv::Point2d(-1.0, -1.0);
	
	int smallestBallArea = 4;
	cv::Point2d center(-1, -1);

	cv::Mat imgThresholded = HSVRanges[target]; // reference counted, I think
	if (imgThresholded.rows == 0){
		std::cout << "Image thresholding has failed" << std::endl;
		return;
	}
	cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

	std::vector<std::vector<cv::Point>> contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;

	cv::Scalar color(0, 0, 0);
	cv::Scalar color2(255, 255, 255);
	cv::Scalar redColor(0, 0, 255);

	findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image

	if (contours.size() == 0){ //if no contours found
		return;
	}

	for (int i = 0; i < contours.size(); i++)
	{
		int ballArea = cv::contourArea(contours[i], false);
		if (ballArea >= smallestBallArea) {
			cv::Moments M = cv::moments(contours[i]);
			int posY = M.m01 / M.m00;
			int posX = M.m10 / M.m00;
			cv::Rect bounding_rect = cv::boundingRect(contours[i]);
			rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), redColor, 1, 8, 0);
		
			fieldState.balls[i].load().updateCoordinates(posX, posY);
		}
	}
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
