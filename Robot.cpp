#include "robot.h"
#include "colorcalibrator.h"
#include "autocalibrator.h"
#include "camera.h"
#include "stillcamera.h"
#include "wheelcontroller.h"
#include "coilBoard.h"
#include "GateFinder.h"
#include "BallFinder.h"
#include "dialog.h"
#include "wheel.h"
#include "ComPortScanner.h"
#include "Arduino.h"

#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/filesystem.hpp>
#include "AutoPilot.h"
#include "NewAutoPilot.h"
#include "RobotTracker.h"
#include "ImageThresholder.h"
#include "VideoRecorder.h"

#define STATE_BUTTON(name, new_state) \
createButton(std::string("") + name, [&](){ this->SetState(new_state); });
#define BUTTON(name, function_body) \
createButton(name, [&]() function_body);
#define START_DIALOG if (state != last_state) { \
clearButtons();
#define END_DIALOG } \
last_state = (STATE)state; 


std::pair<OBJECT, std::string> objects[] = {
	std::pair<OBJECT, std::string>(BALL, "Ball"),
	std::pair<OBJECT, std::string>(GATE1, "Blue Gate"),
	std::pair<OBJECT, std::string>(GATE2, "Yellow Gate"),
	std::pair<OBJECT, std::string>(FIELD, "Field"),
    std::pair<OBJECT, std::string>(INNER_BORDER, "Inner Border"),
	std::pair<OBJECT, std::string>(OUTER_BORDER, "Outer Border"),
//	std::pair<OBJECT, std::string>(NUMBER_OF_OBJECTS, "") // this is intentionally left out

};

std::map<OBJECT, std::string> OBJECT_LABELS(objects, objects + sizeof(objects) / sizeof(objects[0]));

std::pair<STATE, std::string> states[] = {
	std::pair<STATE, std::string>(STATE_NONE, "None"),
	std::pair<STATE, std::string>(STATE_AUTOCALIBRATE, "Autocalibrate"),
	std::pair<STATE, std::string>(STATE_CALIBRATE, "Manual calibrate"),
	std::pair<STATE, std::string>(STATE_LAUNCH, "Launch"),
	std::pair<STATE, std::string>(STATE_SETTINGS, "Settings"),
//	std::pair<STATE, std::string>(STATE_LOCATE_GATE, "Locate Gate"),
	std::pair<STATE, std::string>(STATE_REMOTE_CONTROL, "Remote Control"),
//	std::pair<STATE, std::string>(STATE_CRASH, "Crash"),
	std::pair<STATE, std::string>(STATE_RUN, "Autopilot"),
	std::pair<STATE, std::string>(STATE_TEST, "Test"),
	std::pair<STATE, std::string>(STATE_MANUAL_CONTROL, "Manual Control"),
	std::pair<STATE, std::string>(STATE_TEST_COILGUN, "Test CoilGun"),
	std::pair<STATE, std::string>(STATE_SELECT_GATE, "Select Gate"),
	std::pair<STATE, std::string>(STATE_DANCE, "Dance"),
	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};

std::map<STATE, std::string> STATE_LABELS(states, states + sizeof(states) / sizeof(states[0]));

/* BEGIN DANCE MOVES */
void dance_step(float time, float &move1, float &move2) {
	move1 = 50*sin(time/4000);
	move2 = 360 * cos(time / 4000);
}

/* END DANCE MOVES */

Robot::Robot(boost::asio::io_service &io) : Dialog("Robotiina"), io(io), camera(0), wheels(0), coilBoard(0),arduino(0)
{
	
	last_state = STATE_END_OF_GAME;
	state = STATE_NONE;
    //wheels = new WheelController(io);
	assert(OBJECT_LABELS.size() == NUMBER_OF_OBJECTS);
	assert(STATE_LABELS.size() == STATE_END_OF_GAME);
	autoPilotEnabled = false;
}
Robot::~Robot()
{
	if (camera)
		delete camera;
	std::cout << "coilBoard " << coilBoard << std::endl;
	if (coilBoard)
		delete coilBoard;
	std::cout << "wheels " << wheels << std::endl;
	if(wheels)
        delete wheels;
	std::cout << "arduino " << arduino << std::endl;
	if (arduino)
		delete arduino;

}

bool Robot::Launch(int argc, char* argv[])
{
	if (!ParseOptions(argc, argv)) return false;

	std::cout << "Initializing camera... " << std::endl;
	if (config.count("camera"))
		camera = new Camera(config["camera"].as<std::string>());
	else
		camera = new Camera(0);
	std::cout << "Done" << std::endl;

	wheels = new WheelController();
	captureFrames = config.count("capture-frames") > 0;
	bool portsOk = false;
	if (config.count("skip-ports")) {
		std::cout << "Skiping COM port check" << std::endl;
		portsOk = true;
	}
	else {
		std::cout << "Checking COM ports... " << std::endl;
		{ // new scope for scanner variable
			ComPortScanner scanner;
			if ((portsOk = scanner.Verify(io)) == false){
				std::cout << "Chek failed, rescanning all ports" << std::endl;
				portsOk = scanner.Scan(io);
			}
		}
		std::cout << "Done" << std::endl;
	}
	if (portsOk) {
		std::cout << "Initializing Wheels... " << std::endl;
		try {
			wheels->InitWheels(io, config.count("skip-ports") > 0);
			std::cout << "Initializing Coilgun... " << std::endl;
			{
				if (config.count("skip-ports") == 0) {
					using boost::property_tree::ptree;
					ptree pt;
					read_ini("conf/ports.ini", pt);
					std::string port = pt.get<std::string>(std::to_string(ID_COILGUN));

					coilBoard = new CoilBoard(io, port);
				}
				else {
					coilBoard = new CoilGun();
				}
	
			}
		}
		catch (...) {
                throw;
			//throw std::runtime_error("error to open wheel port(s)");
		}
		std::cout << "Done" << std::endl;
		std::cout << "Initializing Arduino... " << std::endl;
				if (config.count("skip-ports") == 0) {
					ComPortScanner scanner2;
					if ((portsOk = scanner2.VerifyObject(io, "arduino", ID_AUDRINO)) == false){
						std::cout << "Chek failed, rescanning all ports" << std::endl;
						portsOk = scanner2.ScanObject(io, "arduino", ID_AUDRINO);
					}
					if (portsOk) {
						using boost::property_tree::ptree;
						ptree pt;
						read_ini("conf/arduino.ini", pt);
						std::string port2 = pt.get<std::string>(std::to_string(ID_AUDRINO));

						arduino = new ArduinoBoard(io, port2);
					} else {
						arduino = new Arduino();
					}
				}
				else {
					coilBoard = new CoilGun();
					arduino = new Arduino();
				}
		std::cout << "Done" << std::endl;
		
	}
	else {
		throw std::runtime_error("unable find wheels use \"--skip-ports\" parameter to launch without wheels ");
	}

	std::cout << "Starting Robot" << std::endl;
    Run();
	return true;
}

void Robot::Run()
{
	double fps = 0;
	int frames = 0;
	//timer for rotation measure
	boost::posix_time::ptime lastStepTime;	
	boost::posix_time::time_duration dt;	

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime epoch = boost::posix_time::microsec_clock::local_time();

	boost::posix_time::ptime rotateTime = time;
	boost::posix_time::time_duration rotateDuration;
	cv::Mat frameBGR, frameHSV;

	std::string captureDir;
	boost::posix_time::ptime captureStart = boost::posix_time::microsec_clock::local_time();
	cv::VideoWriter *outputVideo = NULL;

	/*= "videos/" + boost::posix_time::to_simple_string(time) + "/";
	std::replace(captureDir.begin(), captureDir.end(), ':', '.');
	if (captureFrames) {
		boost::filesystem::create_directories(captureDir);
	}
	*/
	NewAutoPilot autoPilot(wheels, coilBoard, arduino);

	//RobotTracker tracker(wheels);
	ThresholdedImages thresholdedImages;
	ImageThresholder thresholder(thresholdedImages, objectThresholds);
	GateFinder gate1Finder;
	GateFinder gate2Finder;
	BallFinder finder;


	frameBGR = camera->Capture();
	
	std::stringstream subtitles;

	bool gaussianBlurEnabled = false;
	bool sonarsEnabled = false;
	bool greenAreaDetectionEnabled = false;
	bool gateObstructionDetectionEnabled = false;
	bool borderDetectionEnabled = false;
	bool nightVisionEnabled = false;

	using boost::property_tree::ptree;
	try {

		ptree pt;
		read_ini("conf/settings.ini", pt);
		gaussianBlurEnabled = pt.get<bool>("gaussianBlur");
		sonarsEnabled = pt.get<bool>("sonars");
		greenAreaDetectionEnabled = pt.get<bool>("greenAreaDetection");
		gateObstructionDetectionEnabled = pt.get<bool>("gateObstructionDetection");
		borderDetectionEnabled = pt.get<bool>("borderDetection");
		nightVisionEnabled = pt.get<bool>("nightVision");
	}
	catch (...){
		ptree pt;
		pt.put("gaussianBlur", false);
		pt.put("sonars", false);
		pt.put("greenAreaDetection", false);
		pt.put("gateObstructionDetection", false);
		pt.put("borderDetection", false);
		pt.put("nightVision", false);
		write_ini("conf/settings.ini", pt);
	};


	/* Input */
	bool ballInTribbler = false;
	cv::Point3i sonars = {100,100,100};
	bool somethingOnWay = false;
	int mouseControl = 0;
	ObjectPosition borderDistance = { INT_MAX, 0, 0 };
	bool notEnoughtGreen = false;
	bool sightObstructed = false;

	try {
		CalibrationConfReader calibrator;
		for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
			objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
		}
	}
	catch (...){
		std::cout << "Calibration data is missing!" << std::endl;

	}

	cv::Mat white(frameBGR.rows, frameBGR.cols, frameBGR.type(), cv::Scalar(255, 255, 255));
	cv::Mat black(frameBGR.rows, frameBGR.cols, frameBGR.type(), cv::Scalar(40, 40, 40));
	cv::Mat green(frameBGR.rows, frameBGR.cols, frameBGR.type(), cv::Scalar(21, 188, 80));
	cv::Mat yellow(frameBGR.rows, frameBGR.cols, frameBGR.type(), cv::Scalar(61, 255, 244));
	cv::Mat blue(frameBGR.rows, frameBGR.cols, frameBGR.type(), cv::Scalar(236, 137, 48));
	cv::Mat orange(frameBGR.rows, frameBGR.cols, frameBGR.type(), cv::Scalar(48, 154, 236));

	cv::Mat display_empty(frameBGR.rows + 160, frameBGR.cols + 200, frameBGR.type(), cv::Scalar(0));
	cv::Mat display(frameBGR.rows + 160, frameBGR.cols + 200, frameBGR.type(), cv::Scalar(0));
	cv::Mat display_roi = display(cv::Rect(0, 0, frameBGR.cols, frameBGR.rows)); // region of interest
	VideoRecorder videoRecorder("videos/", 30, display.size());
	AutoCalibrator calibrator(frameBGR.size());

	while (true)
    {
		if (last_state == STATE_SETTINGS) {
			ptree pt;
			pt.put("gaussianBlur", gaussianBlurEnabled);
			pt.put("sonars", sonarsEnabled);
			pt.put("greenAreaDetection", greenAreaDetectionEnabled);
			pt.put("gateObstructionDetection", gateObstructionDetectionEnabled);
			pt.put("borderDetection", borderDetectionEnabled);
			pt.put("nightVision", nightVisionEnabled);
			write_ini("conf/settings.ini", pt);
		}
		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type dt = (time - lastStepTime).total_milliseconds();
		boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();

		if (dt > 1000) {
			fps = 1000.0 * frames / dt;
			lastStepTime = time;
			frames = 0;
		}
#define RECORD_AFTER_PROCESSING
#ifdef RECORD_AFTER_PROCESSING
		if (captureFrames) {
			videoRecorder.RecordFrame(display, subtitles.str());
		}
#endif
		display_empty.copyTo(display);
		frameBGR = camera->Capture();
		
#ifndef RECORD_AFTER_PROCESSING
		if (captureFrames) {
			videoRecorder.RecordFrame(display, subtitles.str());
		}
#endif
		//
		/**************************************************/
		/*	STEP 1. Convert picture to HSV colorspace	  */
		/**************************************************/

		if (gaussianBlurEnabled) {
			cv::GaussianBlur(frameBGR, frameBGR, cv::Size(3, 3), 4);
		}
		cvtColor(frameBGR, frameHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		if (!nightVisionEnabled || state == STATE_AUTOCALIBRATE) {
			if (state == STATE_AUTOCALIBRATE) {
				cv::Mat mask(frameBGR.rows, frameBGR.cols, CV_8U, cv::Scalar::all(0));
				frameBGR.copyTo(display_roi, calibrator.mask);
			}
			else {
				frameBGR.copyTo(display_roi);
			}
		}
		/**************************************************/
		/*	STEP 2. thresholding in parallel	          */
		/**************************************************/
		thresholder.Start(frameHSV, { BALL, GATE1, GATE2, INNER_BORDER, OUTER_BORDER, FIELD });
		thresholder.WaitForStop();

		/* STEP 2.2 cover own balls */
		std::vector<cv::Point2i> triangle;
		triangle.push_back(cv::Point(100, frameBGR.rows - 50));
		triangle.push_back(cv::Point(230, frameBGR.rows - 60));
		triangle.push_back(cv::Point(240, frameBGR.rows));
		triangle.push_back(cv::Point(0, frameBGR.rows));
		cv::fillConvexPoly(thresholdedImages[BALL], triangle, cv::Scalar::all(0));
		cv::fillConvexPoly(display_roi, triangle, cv::Scalar(255,0,255));
		triangle.clear();
		triangle.push_back(cv::Point(frameBGR.cols - 100, frameBGR.rows - 50));
		triangle.push_back(cv::Point(frameBGR.cols - 230, frameBGR.rows - 60));
		triangle.push_back(cv::Point(frameBGR.cols - 240, frameBGR.rows));
		triangle.push_back(cv::Point(frameBGR.cols - 0, frameBGR.rows));
		cv::fillConvexPoly(thresholdedImages[BALL], triangle, cv::Scalar::all(0));
		cv::fillConvexPoly(display_roi, triangle, cv::Scalar(255,0,255));

		/**************************************************/
		/*	STEP 3. check that path to gate is clean      */
		/* this is done here, because finding contures	  */
		/* corrupts thresholded imagees					  */
		/**************************************************/
		sightObstructed = false;
		if (gateObstructionDetectionEnabled) {
			cv::Mat selected(frameBGR.rows, frameBGR.cols, CV_8U, cv::Scalar::all(0));
			cv::Mat mask(frameBGR.rows, frameBGR.cols, CV_8U, cv::Scalar::all(0));
			cv::Mat	tmp(frameBGR.rows, frameBGR.cols, CV_8U, cv::Scalar::all(0));
			//cv::line(mask, cv::Point(frameBGR.cols / 2, 200), cv::Point(frameBGR.cols / 2 - 40, frameBGR.rows - 100), cv::Scalar(255, 255, 255), 40);
			//cv::line(mask, cv::Point(frameBGR.cols / 2, 200), cv::Point(frameBGR.cols / 2 + 40, frameBGR.rows - 100), cv::Scalar(255, 255, 255), 40);
			std::vector<cv::Point2i> triangle;
			triangle.push_back(cv::Point(frameBGR.cols / 2, 50));
			triangle.push_back(cv::Point(frameBGR.cols / 2 - 80, frameBGR.rows - 100));
			triangle.push_back(cv::Point(frameBGR.cols / 2 + 80, frameBGR.rows - 100));
			cv::fillConvexPoly(mask, triangle, cv::Scalar::all(255));
 			tmp = 255 - (thresholdedImages[INNER_BORDER] + thresholdedImages[OUTER_BORDER] + thresholdedImages[FIELD]);
			tmp.copyTo(selected, mask); // perhaps use field and inner border
			thresholdedImages[SIGHT_MASK] = selected;
			//sightObstructed = countNonZero(selected) > 10;
		}
		// copy thresholded images before they are destroyed
		if (nightVisionEnabled && state != STATE_AUTOCALIBRATE) {
			green.copyTo(display_roi, thresholdedImages[FIELD]);
			white.copyTo(display_roi, thresholdedImages[INNER_BORDER]);
			black.copyTo(display_roi, thresholdedImages[OUTER_BORDER]);
			orange.copyTo(display_roi, thresholdedImages[BALL]);
			yellow.copyTo(display_roi, thresholdedImages[GATE2]);
			blue.copyTo(display_roi, thresholdedImages[GATE1]);
		}
	
		if (borderDetectionEnabled) {
			float y = finder.IsolateField(thresholdedImages, frameHSV, display_roi, false, nightVisionEnabled);
			finder.LocateCursor(display_roi, cv::Point2i(frameBGR.cols /2, y), BALL, borderDistance);
		}
		else {
			borderDistance = { INT_MAX, 0, 0 };
		};

		/**************************************************/
		/* STEP 4. extract closest ball and gate positions*/
		/**************************************************/
		ObjectPosition ballPos, gate1Pos, gate2Pos;
		//Cut out gate contour.	

		bool gate1Found = gate2Finder.Locate(thresholdedImages, frameHSV, display_roi, GATE1, gate1Pos);
		bool gate2Found = gate1Finder.Locate(thresholdedImages, frameHSV, display_roi, GATE2, gate2Pos);

		bool ballFound = mouseControl != 1 ?
			finder.Locate(thresholdedImages, frameHSV, display_roi, BALL, ballPos)
			: finder.LocateCursor(display_roi, cv::Point2i(mouseX, mouseY), BALL, ballPos);

		ObjectPosition *targetGatePos = 0;
		if (targetGate == GATE1 && gate1Found) targetGatePos = &gate1Pos;
		else if (targetGate == GATE2 && gate2Found) targetGatePos = &gate2Pos;
		// else leave to NULL

		if (gateObstructionDetectionEnabled) {
			// step 3.2
			int count = countNonZero(thresholdedImages[SIGHT_MASK]);
			std::ostringstream osstr;
			osstr << "nonzero :" << count;
			sightObstructed = count > 900;
			//cv::putText(thresholdedImages[SIGHT_MASK], osstr.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			//cv::imshow("mmm", thresholdedImages[SIGHT_MASK]);
		}

		/**************************************************/
		/* STEP 5. check if ball is in tribbler			  */
		/**************************************************/
		ballInTribbler = coilBoard->BallInTribbler();
		/**************************************************/
		/* STEP 6. check if something is in front of us   */
		/**************************************************/
		sonars = { 100, 100, 100 };
		if (sonarsEnabled) {
			sonars = arduino->GetSonarReadings();
			somethingOnWay = (
				(sonars.x < 10 && sonars.x > 0) ||
				(sonars.y < 10 && sonars.y > 0) ||
				(sonars.z < 10 && sonars.z > 0));
		}
		notEnoughtGreen = false;
		if (greenAreaDetectionEnabled) {
			notEnoughtGreen = countNonZero(thresholdedImages[FIELD]) < 640 * 120;
			somethingOnWay |= notEnoughtGreen;
		}
		// step 6.9
		cv::Point2i ballCount(finder.ballCountLeft, finder.ballCountRight);
		/**************************************************/
		/* STEP 7. feed these variables to Autopilot	  */
		/**************************************************/
		std::ostringstream oss;
		oss.precision(4);

		oss << "[Robot] State: " << STATE_LABELS[state];
		oss << ", Ball: " << (ballFound ? "yes" : "no");
		oss << ", Gate: " << (targetGatePos != NULL ? "yes" : "no");
		oss << ", trib: " << (ballInTribbler ? "yes" : "no");
		oss << ", Sight: " << (!sightObstructed ? "yes" : "no");
		oss << "|[Robot] Ball Pos: (" << ballPos.distance << "," << ballPos.horizontalAngle << "," << ballPos.horizontalDev << ")";
		if(targetGatePos != NULL)
			oss << "|[Robot] Gate Pos: (" << targetGatePos->distance << "," << targetGatePos->horizontalAngle << "," << targetGatePos->horizontalDev << ")";
		else
			oss << "|[Robot] Gate Pos: - ";
//		oss << "Gate Pos: (" << lastBallLocation.distance << "," << lastBallLocation.horizontalAngle << "," << lastBallLocation.horizontalDev << ")";


		if (autoPilotEnabled || autoPilot.testMode) {
			autoPilot.UpdateState(ballFound ? &ballPos : NULL, targetGatePos, ballInTribbler, sightObstructed, somethingOnWay, borderDistance.distance, ballCount);			
		}
		
		//---------------------------------------------
		
		if (sonarsEnabled) { // TODO: make new flag
			int gate = arduino->getGate();
			int start = arduino->getStart();
			if (gate > -1) {
				OBJECT newGate = gate == 0 ? GATE1 : GATE2;
				if(newGate != targetGate){
					targetGate = newGate;
					last_state = STATE_END_OF_GAME;
				}
			}
			if (start == 1) {
				autoPilotEnabled = !autoPilotEnabled;
				last_state = STATE_END_OF_GAME;
			}
		}
		/**************************************************/
		/* STEP 8. kick and drive (done in AutoPilot	  */
		/**************************************************/

		/* Main UI */
		if (STATE_NONE == state) {
			START_DIALOG
				autoPilot.enableTestMode(false);
				wheels->Stop();
				STATE_BUTTON("(A)utoCalibrate objects", STATE_AUTOCALIBRATE)
				//STATE_BUTTON("(M)anualCalibrate objects", STATE_CALIBRATE)
				STATE_BUTTON("(C)Change Gate [" + OBJECT_LABELS[targetGate] + "]", STATE_SELECT_GATE)
				STATE_BUTTON("Auto(P)ilot [" + (autoPilotEnabled ? "On" : "Off") + "]", STATE_LAUNCH)

			createButton(std::string("(M)ouse control [") + (mouseControl == 0 ? "Off" : (mouseControl == 1 ? "Ball" : "Gate")) + "]", [this, &mouseControl, &frameBGR]{
				mouseControl = (mouseControl + 1) % 3;
				this->last_state = STATE_END_OF_GAME; // force dialog redraw
			});
			//				STATE_BUTTON("(D)ance", STATE_DANCE)
				//STATE_BUTTON("(D)ance", STATE_DANCE)
				//STATE_BUTTON("(R)emote Control", STATE_REMOTE_CONTROL)
				createButton(std::string("Save video: ") + (captureFrames ? "on" : "off"), [this, &captureDir, &time, &videoRecorder, &frameBGR]{
					if (this->captureFrames) {
						// save old video
					}

					this->captureFrames = !this->captureFrames;
					this->captureFrames ? videoRecorder.Start() : videoRecorder.Stop();

					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				STATE_BUTTON("Settings", STATE_SETTINGS)
				STATE_BUTTON("Manual (C)ontrol", STATE_MANUAL_CONTROL)
				STATE_BUTTON("(T)est CoilGun", STATE_TEST_COILGUN)
				STATE_BUTTON("(T)est Autopilot", STATE_TEST)
				STATE_BUTTON("E(x)it", STATE_END_OF_GAME)
			END_DIALOG
		}
		else if (STATE_AUTOCALIBRATE == state) {
			START_DIALOG
				calibrator.reset();
				createButton("Take a screenshot", [this, &frameBGR, &calibrator]{
					if (calibrator.LoadFrame(frameBGR)) {
						this->SetState(STATE_CALIBRATE);
					};

				});
			STATE_BUTTON("BACK", STATE_NONE)
			END_DIALOG
		}
		else if (STATE_CALIBRATE == state) {
			START_DIALOG
				for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
					// objectThresholds[(OBJECT) i] = calibrator->GetObjectThresholds(i, OBJECT_LABELS[(OBJECT) i]);
					createButton(OBJECT_LABELS[(OBJECT)i], [this, i, &frameBGR, &calibrator]{
						this->objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					});
				}
				STATE_BUTTON("BACK", STATE_NONE)
			END_DIALOG
		}
		else if (STATE_SELECT_GATE == state) {
			START_DIALOG
				createButton(OBJECT_LABELS[GATE1], [this]{
				this->targetGate = GATE1;
				this->SetState(STATE_NONE);
			});
			createButton(OBJECT_LABELS[GATE2], [this]{
				this->targetGate = GATE2;
				this->SetState(STATE_NONE);
			});
			END_DIALOG
		}
		else if (STATE_SETTINGS == state) {
			START_DIALOG
				createButton(std::string("Gaussian blur: ") + (gaussianBlurEnabled ? "on" : "off"), [this, &gaussianBlurEnabled]{
					gaussianBlurEnabled = !gaussianBlurEnabled;
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				createButton(std::string("Night vision: ") + (nightVisionEnabled ? "on" : "off"), [this, &nightVisionEnabled]{
					nightVisionEnabled = !nightVisionEnabled;
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				createButton(std::string("Sonars: ") + (sonarsEnabled ? "on" : "off"), [this, &sonarsEnabled]{
					sonarsEnabled = !sonarsEnabled;
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				createButton(std::string("Border Detection: ") + (borderDetectionEnabled ? "on" : "off"), [this, &borderDetectionEnabled]{
					borderDetectionEnabled = !borderDetectionEnabled;
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				createButton(std::string("Field detection: ") + (greenAreaDetectionEnabled ? "on" : "off"), [this, &greenAreaDetectionEnabled]{
					greenAreaDetectionEnabled = !greenAreaDetectionEnabled;
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				createButton(std::string("Gate obstructed det.: ") + (gateObstructionDetectionEnabled ? "on" : "off"), [this, &gateObstructionDetectionEnabled]{
					gateObstructionDetectionEnabled = !gateObstructionDetectionEnabled;
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				STATE_BUTTON("BACK", STATE_NONE)
			END_DIALOG
		}
		else if (STATE_LAUNCH == state) {
			if (targetGate == NUMBER_OF_OBJECTS) {
				std::cout << "Select target gate" << std::endl;
				SetState(STATE_SELECT_GATE);
			}
			else {
				try {
					CalibrationConfReader calibrator;
					for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
						objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					}
					//SetState(STATE_SELECT_GATE);
					autoPilotEnabled = !autoPilotEnabled;
					if (!autoPilotEnabled) {
						coilBoard->ToggleTribbler(false);
						wheels->Stop();
					}
					SetState(STATE_NONE);
				}
				catch (...){
					std::cout << "Calibration data is missing!" << std::endl;
					//TODO: display error
					SetState(STATE_AUTOCALIBRATE); // no conf
				}
			}
		}
		else if (STATE_RUN == state) {
			START_DIALOG
				createButton(std::string("Save video: ") + (captureFrames ? "on" : "off"), [this, &captureDir, &time, &videoRecorder, &frameBGR]{
					if (this->captureFrames) {
						// save old video
					}

					this->captureFrames = !this->captureFrames;
					this->captureFrames ? videoRecorder.Start() : videoRecorder.Stop();

					this->last_state = STATE_NONE; // force dialog redraw
				});
				/*
				createButton(std::string("Mouse control: ") + (dynamic_cast<MouseFinder*>(finder) == NULL ? "off" : "on"), [this]{
					bool isMouse = dynamic_cast<MouseFinder*>(finder) != NULL;
					delete this->finder;
					this->finder = isMouse ? new ObjectFinder() : new MouseFinder();
					this->last_state = STATE_NONE;
				});
				*/
				STATE_BUTTON("(B)ack", STATE_NONE)
				STATE_BUTTON("E(x)it", STATE_END_OF_GAME)
			END_DIALOG

				
			
//			autoPilot->UpdateState(ballFound ? &ballPos : NULL, targetGatePos, sightObstructed);
			
        }
		else if (STATE_TEST == state) {
			START_DIALOG
				autoPilot.enableTestMode(true);
				for (const auto d : autoPilot.driveModes) {
					createButton(d.second->name, [this, &frameBGR, &calibrator, &autoPilot, d]{
						autoPilot.setTestMode(d.first);
					});
				}
			STATE_BUTTON("BACK", STATE_NONE)
				END_DIALOG
		}
		else if (STATE_MANUAL_CONTROL == state) {
			START_DIALOG
				createButton("Move Left", [this] {this->wheels->Drive(40, 90); });
				createButton("Move Right", [this]{this->wheels->Drive(40, -90); });
				createButton("Move Forward", [this]{this->wheels->Drive(190, 0); });
				createButton("Move Back", [this]{this->wheels->Drive(-40, 0); });
				createButton("Rotate Right", [this]{this->wheels->Rotate(0, 20); });
				createButton("Rotate Left", [this]{this->wheels->Rotate(1, 20); });
				STATE_BUTTON("Back", STATE_NONE)
			END_DIALOG
		}
		else if (STATE_TEST_COILGUN == state) {
			START_DIALOG
				createButton("Kick", [this] {this->coilBoard->Kick(); });
				createButton("Start tribbler", [this]{this->coilBoard->ToggleTribbler(true); });
				createButton("Stop tribbler", [this]{this->coilBoard->ToggleTribbler(false); });
				STATE_BUTTON("Back", STATE_NONE)
			END_DIALOG
		}


		else if (STATE_DANCE == state) {
			float move1, move2;
			dance_step(((float)(time - epoch).total_milliseconds()), move1, move2);
			wheels->Drive(move1, move2);
			cv::putText(frameBGR, "move1:" + std::to_string(move1), cv::Point(frameHSV.cols - 140, 120), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			cv::putText(frameBGR, "move2:" + std::to_string(move2), cv::Point(frameHSV.cols - 140, 140), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		}
		else if (STATE_END_OF_GAME == state) {
			break;
		}
		 
		subtitles.str("");
		//subtitles << oss.str();
		subtitles << "|" << autoPilot.GetDebugInfo();
		subtitles << "|" << wheels->GetDebugInfo();
		subtitles << "|" << arduino->GetDebugInfo();


		cv::putText(display, "fps: " + std::to_string(fps), cv::Point(display.cols - 140, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		//assert(STATE_END_OF_GAME != state);
		cv::putText(display, "state: " + STATE_LABELS[state], cv::Point(display.cols - 140, 40), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		cv::putText(display, std::string("Ball:") + (ballFound ? "yes" : "no"), cv::Point(display.cols - 140, 60), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, std::string("Gate:") + (targetGatePos != NULL ? "yes" : "no"), cv::Point(display.cols - 140, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, std::string("Trib:") + (ballInTribbler ? "yes" : "no"), cv::Point(display.cols - 140, 100), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, std::string("Sight:") + (sightObstructed ? "obst" : "free"), cv::Point(display.cols - 140, 120), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, std::string("OnWay:") + (somethingOnWay ? "yes" : "no"), cv::Point(display.cols - 140, 140), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		cv::putText(display, "Ball" , cv::Point(display.cols - 140, 180), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, "dist: " + std::to_string(ballPos.distance), cv::Point(display.cols - 140, 200), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, "angle :" + std::to_string(ballPos.horizontalAngle), cv::Point(display.cols - 140, 220), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, "dev: " + std::to_string(ballPos.horizontalDev), cv::Point(display.cols - 140, 240), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		cv::putText(display, "border: " + std::to_string(borderDistance.distance), cv::Point(display.cols - 140, 280), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		if (targetGatePos != NULL) {
			cv::putText(display, "Gate" ,  cv::Point(display.cols - 140, 320), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			cv::putText(display, "dist: " + std::to_string(targetGatePos->distance), cv::Point(display.cols - 140, 340), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			cv::putText(display, "angle: " + std::to_string(targetGatePos->horizontalAngle), cv::Point(display.cols - 140, 360), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			cv::putText(display, "dev: " + std::to_string(targetGatePos->horizontalDev), cv::Point(display.cols - 140, 380), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		}
		else {
			cv::putText(display, "Gate - N/A", cv::Point(display.cols - 140, 320), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		}

		//TODO: fix putText newline thing
		std::vector<std::string> subtitles2;
		std::string subtitles3 = subtitles.str();

		boost::split(subtitles2, subtitles3, boost::is_any_of("|"));

		int j = 0;
		for (auto s : subtitles2) {
			cv::putText(display, s, cv::Point(10, display.rows - 150 + j), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			j += 20;
		}

		/* robot tracker */
		cv::Point2i center(display.cols - 100, 200);
		double velocity = 0, direction = 0, rotate = 0;
		auto speed = wheels->GetTargetSpeed();

		/*
		//Draw circle
		cv::Scalar colorCircle(133, 33, 55);
		cv::circle(display, center, 60, colorCircle, 2);
		*/
		show(display);
		if (cv::waitKey(1) == 27) {
			std::cout << "exiting program" << std::endl;
			break;
		}
		frames++;

    }
    	
	if (outputVideo != NULL) {
		delete outputVideo;
	}


}


std::string Robot::ExecuteRemoteCommand(const std::string &command){
    std::stringstream response;
    boost::mutex::scoped_lock lock(remote_mutex); //allow one command at a time
    std::vector<std::string> tokens;
    boost::split(tokens, command, boost::is_any_of(";"));
    std::string query = tokens[0];
    STATE s = (STATE)GetState();
    if(query == "status") response << s;
    else if(query == "remote") SetState(STATE_REMOTE_CONTROL);
    else if(query == "cont") SetState(STATE_RUN);
    else if(query == "reset") SetState(STATE_NONE);
    else if(query == "exit") SetState(STATE_END_OF_GAME);
    else if (STATE_REMOTE_CONTROL == s) {
        if (query == "drive" && tokens.size() == 3) {
            int speed = atoi(tokens[1].c_str());
			double direction = atof(tokens[2].c_str());
            wheels->Drive(speed, direction);
		}
		else if (query == "rdrive" && tokens.size() == 4) {
			int speed = atoi(tokens[1].c_str());
			double direction = atof(tokens[2].c_str());
			int rotate = atoi(tokens[3].c_str());
			wheels->DriveRotate(speed, direction, rotate);
		}

    }
    return response.str();
}

bool Robot::ParseOptions(int argc, char* argv[])
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("camera", po::value<std::string>(), "set camera index or path")
		("locate_cursor", "find cursor instead of ball")
		("skip-ports", "skip COM port checks")
		("save-frames", "Save captured frames to disc");

	po::store(po::parse_command_line(argc, argv, desc), config);
	po::notify(config);

	if (config.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	return true;
}
