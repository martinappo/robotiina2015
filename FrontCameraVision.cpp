#include "FrontCameraVision.h"
#include "ImageThresholder.h"
#include "GateFinder.h"
#include "BallFinder.h"
#include "AutoCalibrator.h"

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
	ImageThresholder thresholder(thresholdedImages, objectThresholds);
	GateFinder BlueGateFinder;
	GateFinder YellowGateFinder;
	BallFinder ballFinder;

	auto frameSize = m_pCamera->GetFrameSize();

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

		thresholder.Start(frameHSV, { BALL, BLUE_GATE, YELLOW_GATE/*, FIELD, INNER_BORDER, OUTER_BORDER, */ });
		//thresholder.WaitForStop();
		/* STEP 2.2 cover own balls */
		/*
		std::vector<cv::Point2i> triangle;
		triangle.push_back(cv::Point(100, frameSize.height - 50));
		triangle.push_back(cv::Point(230, frameSize.height - 60));
		triangle.push_back(cv::Point(240, frameSize.height));
		triangle.push_back(cv::Point(0, frameSize.height));
		cv::fillConvexPoly(thresholdedImages[BALL], triangle, cv::Scalar::all(0));
		cv::fillConvexPoly(frameBGR, triangle, cv::Scalar(255, 0, 255));
		triangle.clear();
		triangle.push_back(cv::Point(frameSize.width - 100, frameSize.height - 50));
		triangle.push_back(cv::Point(frameSize.width - 230, frameSize.height - 60));
		triangle.push_back(cv::Point(frameSize.width - 240, frameSize.height));
		triangle.push_back(cv::Point(frameSize.width - 0, frameSize.height));
		cv::fillConvexPoly(thresholdedImages[BALL], triangle, cv::Scalar::all(0));
		cv::fillConvexPoly(frameBGR, triangle, cv::Scalar(255, 0, 255));
		*/
		/**************************************************/
		/*	STEP 3. check that path to gate is clean      */
		/* this is done here, because finding contures	  */
		/* corrupts thresholded imagees					  */
		/**************************************************/
		if (gateObstructionDetectionEnabled) {
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
		
		// copy thresholded images before they are destroyed
		if (nightVisionEnabled) {
			green.copyTo(frameBGR, thresholdedImages[FIELD]);
			white.copyTo(frameBGR, thresholdedImages[INNER_BORDER]);
			black.copyTo(frameBGR, thresholdedImages[OUTER_BORDER]);
			orange.copyTo(frameBGR, thresholdedImages[BALL]);
			yellow.copyTo(frameBGR, thresholdedImages[YELLOW_GATE]);
			blue.copyTo(frameBGR, thresholdedImages[BLUE_GATE]);
		}
		
		if (borderDetectionEnabled) {
			//float y = finder.IsolateField(thresholdedImages, frameHSV, frameBGR, false, nightVisionEnabled);
			//finder.LocateCursor(frameBGR, cv::Point2i(frameSize.width / 2, y), BALL, borderDistance);
		}
		else {
			borderDistance.setDistance(INT_MAX);
		};

		/**************************************************/
		/* STEP 4. extract closest ball and gate positions*/
		/**************************************************/
		ObjectPosition blueGatePos, yellowGatePos;
		BallPosition ballPos;
		//Cut out gate contour.	

		bool BlueGateFound = BlueGateFinder.Locate(thresholdedImages, frameHSV, frameBGR, BLUE_GATE, blueGatePos);
		bool YellowGateFound = YellowGateFinder.Locate(thresholdedImages, frameHSV, frameBGR, YELLOW_GATE, yellowGatePos);

		bool ballFound = ballFinder.Locate(thresholdedImages, frameHSV, frameBGR, BALL, ballPos);

		ballFinder.PopulateBalls(thresholdedImages, frameHSV, frameBGR, BALL, m_pState);

		m_pState->blueGate = blueGatePos;
		m_pState->yellowGate = yellowGatePos;
		m_pState->self.load().updateCoordinates(0,0);

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

		m_pDisplay->ShowImage(frameBGR);
		//TODO: update state
	}
}