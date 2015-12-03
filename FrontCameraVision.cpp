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
#include "kdNode2D.h"
#include "DistanceCalculator.h"
#include "VideoRecorder.h"


extern DistanceCalculator gDistanceCalculator;


FrontCameraVision::FrontCameraVision(ICamera *pCamera, IDisplay *pDisplay, FieldState *pFieldState) : ConfigurableModule("FrontCameraVision"), ThreadedClass("FrontCameraVision")
{
	m_pCamera = pCamera;
	m_pDisplay = pDisplay;
	m_pState = pFieldState;

	ADD_BOOL_SETTING(gaussianBlurEnabled);
	ADD_BOOL_SETTING(greenAreaDetectionEnabled);
	ADD_BOOL_SETTING(gateObstructionDetectionEnabled);
	ADD_BOOL_SETTING(borderDetectionEnabled);
	ADD_BOOL_SETTING(borderCollisonEnabled);
	ADD_BOOL_SETTING(fieldCollisonEnabled);
	ADD_BOOL_SETTING(nightVisionEnabled);
	ADD_BOOL_SETTING(detectOtherRobots);
	videoRecorder = new VideoRecorder("videos/", 30, m_pCamera->GetFrameSize(true));
	LoadSettings();
	Start();
}


FrontCameraVision::~FrontCameraVision()
{
	WaitForStop();
	if (videoRecorder != NULL) {
		videoRecorder->Stop();
		delete videoRecorder;
		videoRecorder = NULL;
	}
}
bool FrontCameraVision::captureFrames(){
	return videoRecorder->isRecording;
}

void FrontCameraVision::captureFrames(bool start){
	if (start) {
		videoRecorder->Start();
	}
	else {
		videoRecorder->Stop();
	}
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

	cv::Mat white(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(255, 255, 255));
	cv::Mat black(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(40, 40, 40));
	cv::Mat green(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(21, 188, 80));
	cv::Mat yellow(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(61, 255, 244));
	cv::Mat blue(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(236, 137, 48));
	cv::Mat orange(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(48, 154, 236));

	bool notEnoughtGreen = false;
	int mouseControl = 0;
	bool somethingOnWay = false;

	while (!stop_thread){
		double t2 = (double)cv::getTickCount();
		double dt = (t2 - time) / cv::getTickFrequency();

		cv::Mat frameBGR = m_pCamera->Capture();
		if (videoRecorder->isRecording){
			videoRecorder->RecordFrame(frameBGR, "");
		}
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

		thresholder.Start(frameHSV, { BALL, BLUE_GATE, YELLOW_GATE, FIELD, INNER_BORDER, OUTER_BORDER});

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
		cv::Point2f blueGate[4], yellowGate[4];
		std::vector<cv::Point2i> notBlueGates, notYellowGates;
		cv::Point blueGateCenter, yellowGateCenter;
		//Blue gate pos
		bool blueFound = blueGateFinder.Locate(thresholdedImages[BLUE_GATE], frameHSV, frameBGR, blueGateCenter, blueGate, notBlueGates);
		if (blueFound) {
			cv::Point vertices[4];
			for (int i = 0; i < 4; ++i){
				vertices[i] = blueGate[i];
			}
			cv::fillConvexPoly(thresholdedImages[BALL], vertices, 4, cv::Scalar::all(0));
		}

		//Yellow gate pos
		bool yellowFound = yellowGateFinder.Locate(thresholdedImages[YELLOW_GATE], frameHSV, frameBGR, yellowGateCenter, yellowGate, notYellowGates);
		if (yellowFound) {
			cv::Point vertices[4];
			for (int i = 0; i < 4; ++i){
				vertices[i] = yellowGate[i];
			}
			cv::fillConvexPoly(thresholdedImages[BALL], vertices, 4, cv::Scalar::all(0));
		}

		// ajust gate positions to ..
		// find closest points to opposite gate centre
		if (blueFound && yellowFound) {
			auto min_i1 = 0, min_j1 = 0, min_i2 = 0, min_j2 = 0;
			double min_dist1 = INT_MAX, min_dist2 = INT_MAX;
			for (int i = 0; i < 4; i++){
				double dist1 = cv::norm(blueGate[i] - (cv::Point2f)yellowGateCenter);
				double dist2 = cv::norm(yellowGate[i] - (cv::Point2f)blueGateCenter);
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
			min_i2 = (cv::norm(blueGate[min_i1] - blueGate[next]) > cv::norm(blueGate[min_i1] - blueGate[prev])) ? next : prev;
			next = (min_j1 + 1) % 4;
			prev = (min_j1 + 3) % 4;
			// find longest side
			min_j2 = (cv::norm(yellowGate[min_j1] - yellowGate[next]) > cv::norm(yellowGate[min_j1] - yellowGate[prev])) ? next : prev;
			cv::Scalar color4(0, 0, 0);

			cv::Scalar color2(0, 0, 255);

			cv::Point2d c1 = (blueGate[min_i1] + blueGate[min_i2]) / 2;
			
			(frameBGR, c1, 12, color4, -1, 12, 0);
			cv::Point2d c2 = (yellowGate[min_j1] + yellowGate[min_j2]) / 2;
			circle(frameBGR, c2, 7, color2, -1, 8, 0);

			m_pState->blueGate.updateRawCoordinates(c1-cv::Point2d(frameBGR.size() / 2));
			m_pState->yellowGate.updateRawCoordinates(c2- cv::Point2d(frameBGR.size() / 2));

			m_pState->self.updateFieldCoords(cv::Point2d(0,0), dt);
		}
		else {
			m_pState->self.predict(dt);
			// calculate gates from predicted pos.
			m_pState->blueGate.polarMetricCoords.x = cv::norm(m_pState->self.fieldCoords - m_pState->blueGate.fieldCoords);
			m_pState->blueGate.polarMetricCoords.y = 360 - gDistanceCalculator.angleBetween(cv::Point(0, 1), m_pState->self.fieldCoords - (m_pState->blueGate.fieldCoords)) + m_pState->self.getAngle();
			m_pState->yellowGate.polarMetricCoords.x = cv::norm(m_pState->self.fieldCoords - m_pState->yellowGate.fieldCoords);;
			m_pState->yellowGate.polarMetricCoords.y = 360 - gDistanceCalculator.angleBetween(cv::Point(0, 1), m_pState->self.fieldCoords - (m_pState->yellowGate.fieldCoords)) + m_pState->self.getAngle();
		}

		cv::circle(thresholdedImages[FIELD], cv::Point(frameBGR.size() / 2), 80, 255, -1);
		cv::circle(thresholdedImages[BALL], cv::Point(frameBGR.size() / 2), 50, 0, -1);
		//imshow("tb",thresholdedImages[BALL]);
		//cv::waitKey(1);
		//COLLISION DETECTION ====================================================================================================
		if (borderCollisonEnabled || fieldCollisonEnabled) {
			bool wasCollisionWithBorder = m_pState->collisionWithBorder;
			bool wasCollisionWithUnknown = m_pState->collisionWithUnknown;
			// mask ourself
			//cv::circle(frameBGR, cv::Point(frameBGR.size() / 2), 70, 255, -1);
			cv::bitwise_or(thresholdedImages[INNER_BORDER], thresholdedImages[FIELD], thresholdedImages[FIELD]);
			//imshow("a", thresholdedImages[FIELD]);
			//cv::waitKey(1);
			m_pState->collisionRange = { -1, -1 };
			bool collisionWithBorder = false;
			bool collisonWithUnknown = false;

			for (size_t c/*orner*/ = 0; c < 4; c++) {
				cv::Rect privateZone(0, 0, 100, 100);

				//if (c == 0) privateZone = cv::Rect (0, 0, 100, 100); //c==0
				//else if (c == 1) privateZone = cv::Rect (0, -100, 100, 100); //c==1
				//else if (c == 2) privateZone = cv::Rect(-100, -100, 100, 100); //c==2
				//else if (c == 3) privateZone = cv::Rect(-100, 0, 100, 100); //c==3
				privateZone += cv::Point((c == 0 || c == 1) ? 0 : -1, (c == 0 || c == 3) ? 0 : -1) * 100;
				privateZone += cv::Point(frameBGR.size() / 2);
				cv::Mat roiOuterBorder(thresholdedImages[OUTER_BORDER], privateZone);
				cv::Mat roiField(thresholdedImages[FIELD], privateZone);
				bool cb = borderCollisonEnabled ? cv::countNonZero(roiOuterBorder) > 160 : false;
				bool cu = fieldCollisonEnabled ? cv::countNonZero(roiField) < 9000 : false;
				//if(c==1) {
				//	std::cout << "coll b: " << cv::countNonZero(roiField) << std::endl;
				//}
				if (cb || cu) {
					if (!collisionWithBorder) {// no previous collison
						m_pState->collisionRange.x = c * 90. - 180;
						m_pState->collisionRange.y = c * 90. - 90;
					}
					else if (m_pState->collisionRange.y + 90. < c*90. - 90.){
						m_pState->collisionRange.x = c * 90. - 180;
					}
					else {
						m_pState->collisionRange.y = c * 90. - 90;
					}
					collisionWithBorder |= cb;
					collisonWithUnknown |= cu;
					cv::rectangle(frameBGR, privateZone, cv::Scalar(cb * 64 + cu * 128, 0, 255), 2, 8);

				}
				else {
					cv::rectangle(frameBGR, privateZone, cv::Scalar(155, 255, 155), 2, 8);
				}
				//std::cout << "coll b: " << cv::countNonZero(roiOuterBorder) << std::endl;
			}
			m_pState->collisionWithBorder = collisionWithBorder;
			m_pState->collisionWithUnknown = collisonWithUnknown;
		}
		else {
			m_pState->collisionWithBorder = false;
			m_pState->collisionWithUnknown = false;
		}
		//std::cout << "coll u: " << cv::countNonZero(roiField) << std::endl;
		//imshow("field", roiField);
		//cv::waitKey(1);

		//Balls pos 
//		cv::Mat rotMat = getRotationMatrix2D(cv::Point(0,0), -m_pState->self.getAngle(), 1);
		//cv::Mat balls(3, m_pState->balls.size(), CV_64FC1);
		std::vector<cv::Point2d> balls;
		bool ballsFound = ballFinder.Locate(thresholdedImages[BALL], frameHSV, frameBGR, balls);
		if (ballsFound) {
			//balls.row(0) -= frameBGR.size().width / 2;
			//balls.row(1) -= frameBGR.size().height / 2;
			std::sort(balls.begin(), balls.end(), [](cv::Point2d a, cv::Point2d b)
			{
				return cv::norm(a) < cv::norm(b);
			});
			// validate balls
			cv::Point2i closest;
			for (auto ball : balls) {
				closest = ball;
				if (BallFinder::validateBall(thresholdedImages, ball, frameHSV, frameBGR)){
					break;
				} else {
					cv::Rect bounding_rect = cv::Rect(closest - cv::Point(20, 20) + cv::Point(frameBGR.size() / 2),
						closest + cv::Point(20, 20) + cv::Point(frameBGR.size() / 2));
					rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), cv::Scalar(255, 0, 255), 2, 8, 0);

				}
			}
			cv::Rect bounding_rect = cv::Rect(closest - cv::Point(20, 20) + cv::Point(frameBGR.size() / 2),
				closest + cv::Point(20, 20) + cv::Point(frameBGR.size() / 2));
			rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), cv::Scalar(255, 0, 0), 2, 8, 0);

//			cv::Mat rotatedBalls(balls.size(), balls.type());

//			rotatedBalls = rotMat * balls;
			m_pState->resetBallsUpdateState();
			m_pState->balls.closest.updateRawCoordinates(closest, cv::Point(0,0));
			m_pState->balls.closest.updateFieldCoords(m_pState->self.getFieldPos(), m_pState->self.getAngle());
			m_pState->balls.closest.isUpdated = true;


			/* find balls that are close by */
			/*
			for (int j = 0; j < balls.size(); j++){
				cv::Point2d rawCoords = cv::Point2d(balls.at<double>(0, j), balls.at<double>(1, j));
				// find if ball projection to gate is larger than gate
				// not quite working
				if (blueFound && cv::norm(m_pState->blueGate.rawPixelCoords) < cv::norm(rawCoords)*cos(gDistanceCalculator.angleBetween(m_pState->blueGate.rawPixelCoords, rawCoords) / 180 * CV_PI)){
					rawCoords = { INT_MAX, INT_MAX };
				}
				if (yellowFound && cv::norm(m_pState->yellowGate.rawPixelCoords) < cv::norm(rawCoords)*cos(gDistanceCalculator.angleBetween(m_pState->yellowGate.rawPixelCoords, rawCoords) / 180 * CV_PI)){
					rawCoords = { INT_MAX, INT_MAX };
				}
				m_pState->balls[j].updateRawCoordinates(rawCoords, cv::Point(0, 0));
				m_pState->balls[j].updateFieldCoords(m_pState->self.getFieldPos(), m_pState->self.getAngle());
				m_pState->balls[j].isUpdated = true;
			}
			*/
			//TODO: use returned ball instead of balls.at<double>(0, ball_idx) 
			/*
			int ball_idx = 0;
			m_pState->balls.calcClosest(&ball_idx);
			if (ball_idx >= 0) {
				cv::Rect bounding_rect = cv::Rect(cv::Point(balls.at<double>(0, ball_idx), balls.at<double>(1, ball_idx)) - cv::Point(20, 20) + cv::Point(frameBGR.size() / 2),
					cv::Point(balls.at<double>(0, ball_idx), balls.at<double>(1, ball_idx)) + cv::Point(20, 20) + cv::Point(frameBGR.size() / 2));
				rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), cv::Scalar(255, 0, 0), 2, 8, 0);
			}
			*/
			/*
			m_pState->balls.getClosest(true, &ball_idx);
			bounding_rect = cv::Rect(cv::Point(balls.at<double>(0, ball_idx), balls.at<double>(1, ball_idx)) - cv::Point(30, 30) + cv::Point(frameBGR.size() / 2),
				cv::Point(balls.at<double>(0, ball_idx), balls.at<double>(1, ball_idx)) + cv::Point(30, 30) + cv::Point(frameBGR.size() / 2));
			rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), cv::Scalar(255, 255, 0), 2, 8, 0);
			*/
		}
		/*
		ObjectPosition *targetGatePos = 0;
		if (targetGate == BLUE_GATE && BlueGateFound) targetGatePos = &blueGatePos;
		else if (targetGate == YELLOW_GATE && YellowGateFound) targetGatePos = &yellowGatePos;
		// else leave to NULL
		*/

		//PARTNER POSITION ====================================================================================================
		if (detectOtherRobots) {
			bool ourRobotBlueBottom = (m_pState->robotColor == FieldState::ROBOT_COLOR_YELLOW_UP);

			std::vector<std::pair<cv::Point2i, double>> positionsToDistances; //One of the colors position and according distances
			for (size_t blueIndex = 0; blueIndex < notBlueGates.size(); blueIndex++) {
				for (size_t yellowIndex = 0; yellowIndex < notYellowGates.size(); yellowIndex++) {
					cv::Point2i bluePos = notBlueGates[blueIndex];
					cv::Point2i yellowPos = notYellowGates[yellowIndex];
					double distBetweenYellowBlue = cv::norm(bluePos - yellowPos);
					cv::Point2i frameCenter = cv::Point2i(frameSize.width / 2, frameSize.height / 2); //our robot is in center
					double distBetweenBlueAndRobot = cv::norm(bluePos - frameCenter);
					double distBetweenYellowAndRobot = cv::norm(yellowPos - frameCenter);

					if (distBetweenYellowBlue < 200 &&   //If distance between two colors is great, then it cannot be robot
						//If our robot has bottom blue, then blue distance from robot has to less than yellow
						((ourRobotBlueBottom && distBetweenBlueAndRobot < distBetweenYellowAndRobot)
						//If our robot has not bottom blue, then blue distance from robot has to be greater than yellow
						|| (!ourRobotBlueBottom && distBetweenBlueAndRobot > distBetweenYellowAndRobot))) {
						std::pair<cv::Point2i, double> positionToDistance = std::make_pair(bluePos, distBetweenYellowBlue);
						positionsToDistances.push_back(positionToDistance);
					}
				}
			}
			auto sortFunc = [](std::pair<cv::Point2i, double> posToDis1, std::pair<cv::Point2i, double> posToDis2) { return (posToDis1.second < posToDis2.second); };
			std::sort(positionsToDistances.begin(), positionsToDistances.end(), sortFunc);
			if (positionsToDistances.size() > 0) {
				m_pState->partner.updateRawCoordinates(positionsToDistances[0].first, cv::Point(0, 0));
			}
			else {
				m_pState->partner.updateRawCoordinates(cv::Point(-1, -1), cv::Point(0, 0));
			}
			circle(frameBGR, m_pState->partner.rawPixelCoords, 10, cv::Scalar(0, 0, 255), 2, 8, 0);
		}
		else {
			m_pState->partner.updateRawCoordinates(cv::Point(-1, -1), cv::Point(0, 0));
		}

		//Gate obstruction
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
