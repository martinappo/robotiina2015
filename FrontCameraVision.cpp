#include "FrontCameraVision.h"
#include "SimpleImageThresholder.h"
#include "ThreadedImageThresholder.h"
#include "ParallelImageThresholder.h"
#include "TBBImageThresholder.h"
#include "GateFinder.h"
#include "BallFinder.h"
#include "AutoCalibrator.h"
#include <queue>          // std::priority_queue
#include <functional>     // std::greater

FrontCameraVision::FrontCameraVision(ICamera *pCamera, IDisplay *pDisplay, FieldState *pFieldState) : ConfigurableModule("FrontCameraVision")
{
	m_pCamera = pCamera;
	m_pDisplay = pDisplay;
	m_pState = pFieldState;

	ADD_BOOL_SETTING(gaussianBlurEnabled);
	ADD_BOOL_SETTING(greenAreaDetectionEnabled);
	ADD_BOOL_SETTING(gateObstructionDetectionEnabled);
	ADD_BOOL_SETTING(borderDetectionEnabled);
	ADD_BOOL_SETTING(nightVisionEnabled);
	LoadSettings();
	Start();
}


FrontCameraVision::~FrontCameraVision()
{
	WaitForStop();
}

void FrontCameraVision::Run() {
	ThresholdedImages thresholdedImages;

	try {
		CalibrationConfReader calibrator;
		for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
			objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
		}
	}
	catch (...){
		std::cout << "Calibration data is missing!" << std::endl;

	}
	TBBImageThresholder thresholder(thresholdedImages, objectThresholds);
	GateFinder blueGateFinder;
	GateFinder yellowGateFinder;
	BallFinder ballFinder;

	auto frameSize = m_pCamera->GetFrameSize();

	/*
	m_pState->blueGate.setFrameSize(frameSize);
	m_pState->yellowGate.setFrameSize(frameSize);
	*/
	

	cv::Mat white(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(255, 255, 255));
	cv::Mat black(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(40, 40, 40));
	cv::Mat green(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(21, 188, 80));
	cv::Mat yellow(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(61, 255, 244));
	cv::Mat blue(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(236, 137, 48));
	cv::Mat orange(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(48, 154, 236));

	ObjectPosition borderDistance = {0,0};
	bool notEnoughtGreen = false;
	int mouseControl = 0;
	bool somethingOnWay = false;

	while (!stop_thread){
		cv::Mat frameBGR = m_pCamera->Capture();

		/**************************************************/
		/*	STEP 1. Convert picture to HSV colorspace	  */
		/**************************************************/
		if (gaussianBlurEnabled) {
			cv::GaussianBlur(frameBGR, frameBGR, cv::Size(3, 3), 4);
		}
		cvtColor(frameBGR, frameHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		/**************************************************/
		/*	STEP 2. thresholding in parallel	          */
		/**************************************************/

		thresholder.Start(frameHSV, { BALL, BLUE_GATE, YELLOW_GATE/*, FIELD, INNER_BORDER, OUTER_BORDER*/ });

		/**************************************************/
		/*	STEP 3. check that path to gate is clean      */
		/* this is done here, because finding contures	  */
		/* corrupts thresholded imagees					  */
		/**************************************************/
		if (false && gateObstructionDetectionEnabled) {
			cv::Mat selected(frameSize.height, frameSize.width, CV_8U, cv::Scalar::all(0));
			cv::Mat mask(frameSize.height, frameSize.width, CV_8U, cv::Scalar::all(0));
			cv::Mat	tmp(frameSize.height, frameSize.width, CV_8U, cv::Scalar::all(0));
			//cv::line(mask, cv::Point(frameSize.width / 2, 200), cv::Point(frameSize.width / 2 - 40, frameSize.height - 100), cv::Scalar(255, 255, 255), 40);
			//cv::line(mask, cv::Point(frameSize.width / 2, 200), cv::Point(frameSize.width / 2 + 40, frameSize.height - 100), cv::Scalar(255, 255, 255), 40);
			std::vector<cv::Point2i> triangle;
			triangle.push_back(cv::Point(frameSize.width / 2, 50));
			triangle.push_back(cv::Point(frameSize.width / 2 - 80, frameSize.height - 100));
			triangle.push_back(cv::Point(frameSize.width / 2 + 80, frameSize.height - 100));
			cv::fillConvexPoly(mask, triangle, cv::Scalar::all(255));
			tmp = 255 - (thresholdedImages[INNER_BORDER] + thresholdedImages[OUTER_BORDER] + thresholdedImages[FIELD]);
			tmp.copyTo(selected, mask); // perhaps use field and inner border
			thresholdedImages[SIGHT_MASK] = selected;
			//sightObstructed = countNonZero(selected) > 10;
		}




		/**************************************************/
		/* STEP 4. extract closest ball and gate positions*/
		/**************************************************/
		cv::Point2f r1[4], r2[4];
		cv::Point g1, g2;
		//Blue gate pos
		bool found = blueGateFinder.Locate(thresholdedImages[BLUE_GATE], frameHSV, frameBGR, g1, r1);
		if (!found) continue; // nothing to do :(
		//Yellow gate pos
		found = yellowGateFinder.Locate(thresholdedImages[YELLOW_GATE], frameHSV, frameBGR, g2, r2);
		if (!found) continue; // nothing to do :(
		// ajust gate positions to ..
		// find closest points to opposite gate centre
		auto min_i1 = 0, min_j1 = 0, min_i2 = 0, min_j2 = 0;
		double min_dist1 = INT_MAX, min_dist2 = INT_MAX;
		for (int i = 0; i < 4; i++){
			double dist1 = cv::norm(r1[i] - (cv::Point2f)g2);
			double dist2 = cv::norm(r2[i] - (cv::Point2f)g1);
			if (dist1 < min_dist1) {
				min_dist1 = dist1;
				min_i1 = i;
			}
			if (dist2 < min_dist2) {
				min_dist2 = dist2;
				min_j1 = i;
			}
		}
		auto next = (min_i1 + 1) % 4;
		auto prev = (min_i1 + 3) % 4;
		// find longest side
		min_i2 = (cv::norm(r1[min_i1] - r1[next]) > cv::norm(r1[min_i1] - r1[prev])) ? next : prev;
		next = (min_j1 + 1) % 4;
		prev = (min_j1 + 3) % 4;
		// find longest side
		min_j2 = (cv::norm(r2[min_j1] - r2[next]) > cv::norm(r2[min_j1] - r2[prev])) ? next : prev;
		cv::Scalar color4(0, 0, 0);

		cv::Scalar color2(0, 0, 255);

		auto c1 = (r1[min_i1] + r1[min_i2]) / 2;
		circle(frameBGR, c1, 12, color4, -1, 12, 0);
		auto c2 = (r2[min_j1] + r2[min_j2]) / 2;
		circle(frameBGR, c2, 7, color2, -1, 8, 0);

		m_pState->blueGate.updateRawCoordinates(c1, frameBGR.size() / 2);
		m_pState->yellowGate.updateRawCoordinates(c2, frameBGR.size() / 2);

		m_pState->self.updateCoordinates();
		
		//Balls pos
		cv::Mat rotMat = getRotationMatrix2D(cv::Point(0,0), m_pState->self.getAngle(), 1);
		cv::Mat balls(1, 1, CV_32FC2);
		found = ballFinder.Locate(thresholdedImages[BALL], frameHSV, frameBGR, balls);
		if (!found) continue; // nothing to do :(
		cv::Mat rotatedBalls(balls.size(), balls.type());
		cv::Mat balls_t(balls.size(), balls.type());
		std::cout << CV_64FC1 << ", " << rotMat.type() << ", " << balls.type() << std::endl;
		std::cout << rotMat << std::endl;
		std::cout << balls << std::endl;
//		cv::transpose(balls, balls_t);
//		rotatedBalls = rotMat * balls;
		cv::warpAffine(balls, rotatedBalls, rotMat, balls.size());
		std::cout << rotatedBalls << std::endl;
		for (int i = 0; i < rotatedBalls.cols; i++){
			m_pState->balls[i].updateRawCoordinates(cv::Point(rotatedBalls.at<cv::Vec2f>(0, i)), frameBGR.size() / 2);
		}
		
		/*
		ObjectPosition *targetGatePos = 0;
		if (targetGate == BLUE_GATE && BlueGateFound) targetGatePos = &blueGatePos;
		else if (targetGate == YELLOW_GATE && YellowGateFound) targetGatePos = &yellowGatePos;
		// else leave to NULL
		*/
		if (gateObstructionDetectionEnabled) {
			// step 3.2
			int count = countNonZero(thresholdedImages[SIGHT_MASK]);
			std::ostringstream osstr;
			osstr << "nonzero :" << count;
			m_pState->gateObstructed = count > 900;
			//cv::putText(thresholdedImages[SIGHT_MASK], osstr.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			//cv::imshow("mmm", thresholdedImages[SIGHT_MASK]);
		}
		else {
			m_pState->gateObstructed = false;
		}

		notEnoughtGreen = false;
		if (greenAreaDetectionEnabled) {
			notEnoughtGreen = countNonZero(thresholdedImages[FIELD]) < 640 * 120;
			somethingOnWay |= notEnoughtGreen;
		}

		// copy thresholded images before they are destroyed
		if (nightVisionEnabled) {
			green.copyTo(frameBGR, thresholdedImages[FIELD]);
			white.copyTo(frameBGR, thresholdedImages[INNER_BORDER]);
			black.copyTo(frameBGR, thresholdedImages[OUTER_BORDER]);
			orange.copyTo(frameBGR, thresholdedImages[BALL]);
			yellow.copyTo(frameBGR, thresholdedImages[YELLOW_GATE]);
			blue.copyTo(frameBGR, thresholdedImages[BLUE_GATE]);
		}

		cv::line(frameBGR, (frameSize / 2) + cv::Size(0, -30), (frameSize / 2) + cv::Size(0, 30), cv::Scalar(0, 0, 255), 3, 8, 0);
		cv::line(frameBGR, (frameSize / 2) + cv::Size(-30, 0), (frameSize / 2) + cv::Size(30,0), cv::Scalar(0, 0, 255), 3, 8, 0);
		m_pDisplay->ShowImage(frameBGR);
	}
}