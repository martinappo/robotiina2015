#include "Robot.h"
#include "AutoCalibrator.h"
#include "DistanceCalibrator.h"
#include "DistanceCalculator.h"


#include "Camera.h"
#include "StillCamera.h"
#include "WheelController.h"
#include "CoilBoard.h"
#include "Dialog.h"
#include "WebUI.h"
#include "Wheel.h"
#include "ComPortScanner.h"

#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "NewAutoPilot.h"
#include "RobotTracker.h"
#include "VideoRecorder.h"
#include "FrontCameraVision.h"
#include "ComModule.h"
#include "ManualControl.h"
#include "RemoteControl.h"
#include "SoccerField.h"
#include "MouseVision.h"

#define STATE_BUTTON(name, shortcut, new_state) \
m_pDisplay->createButton(std::string("") + name, shortcut, [&](){ this->SetState(new_state); });
#define BUTTON(name, shortcut, function_body) \
m_pDisplay->createButton(name, shortcut, [&]() function_body);
#define START_DIALOG if (state != last_state) { \
m_pDisplay->clearButtons();
#define END_DIALOG } \
last_state = (STATE)state; 


std::pair<OBJECT, std::string> objects[] = {
	std::pair<OBJECT, std::string>(BALL, "Ball"),
	std::pair<OBJECT, std::string>(BLUE_GATE, "Blue Gate"),
	std::pair<OBJECT, std::string>(YELLOW_GATE, "Yellow Gate"),
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
	std::pair<STATE, std::string>(STATE_MOUSE_VISION, "Mouse Vision"),
	std::pair<STATE, std::string>(STATE_DISTANCE_CALIBRATE, "dist"),
	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};

std::map<STATE, std::string> STATE_LABELS(states, states + sizeof(states) / sizeof(states[0]));

/* BEGIN DANCE MOVES */
void dance_step(float time, float &move1, float &move2) {
	move1 = 50*sin(time/4000);
	move2 = 360 * cos(time / 4000);
}

/* END DANCE MOVES */

Robot::Robot(boost::asio::io_service &io) : io(io), camera(0), wheels(0), coilBoard(0)
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
	if (scanner)
		delete scanner;
	if (m_pDisplay)
		delete m_pDisplay;
}

bool Robot::Launch(int argc, char* argv[])
{
	if (!ParseOptions(argc, argv)) return false;

	// Compose robot from its parts
	if (config.count("webui") == 0)
		m_pDisplay = new Dialog("Robotiina");
	else 
		m_pDisplay = new WebUI(8080);

	if (config.count("skip-ports") == 0) {
		scanner = new ComPortScanner(io);
	}
	wheels = new WheelController(scanner);
	captureFrames = config.count("capture-frames") > 0;
	coilBoardPortsOk = false;
	wheelsPortsOk = false;

	initCamera();
	initPorts();
	initWheels();
	initCoilboard();

	std::cout << "Done initializing" << std::endl;
	std::cout << "Starting Robot" << std::endl;
    Run();
	return true;
}

void Robot::initCamera()
{

	std::cout << "Initializing camera... " << std::endl;
	if (config.count("camera"))
		if (config["camera"].as<std::string>() == "ximea") 
			camera = new Camera(CV_CAP_XIAPI);
		else
			camera = new Camera(config["camera"].as<std::string>());
	else
		camera = new Camera(0);
	std::cout << "Done" << std::endl;
}


void Robot::initPorts()
{
	if (config.count("skip-ports")) {
		std::cout << "Skiping COM port check" << std::endl;
		coilBoardPortsOk = false;
		wheelsPortsOk = false;
	}
	else {
	}
}

void Robot::initWheels()
{
	wheels->Init();
}

void Robot::initCoilboard() {
	if (coilBoardPortsOk) {
		std::cout << "Using coilgun" << std::endl;
		try {
			using boost::property_tree::ptree;
			ptree pt;
			read_ini("conf/ports.ini", pt);
			std::string port = pt.get<std::string>(std::to_string(ID_COILGUN));
			coilBoard = new CoilBoard(io, port);
		}
		catch (...) {
			throw;
		}
	}
	else {
		std::cout << "WARNING: Not using coilgun" << std::endl;
		coilBoard = new CoilGun();
	}
}

void Robot::Run()
{
	//double fps;
	//int frames = 0;
	//timer for rotation measure
	boost::posix_time::ptime lastStepTime;	
	boost::posix_time::time_duration dt;	

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime epoch = boost::posix_time::microsec_clock::local_time();

	boost::posix_time::ptime rotateTime = time;
	boost::posix_time::time_duration rotateDuration;

	std::string captureDir;
	boost::posix_time::ptime captureStart = boost::posix_time::microsec_clock::local_time();
	cv::VideoWriter *outputVideo = NULL;

	/*= "videos/" + boost::posix_time::to_simple_string(time) + "/";
	std::replace(captureDir.begin(), captureDir.end(), ':', '.');
	if (captureFrames) {
		boost::filesystem::create_directories(captureDir);
	}
	*/
	/* Field state */
	SoccerField field(m_pDisplay, camera->GetFrameSize());

	/* Vision modules */
	FrontCameraVision visionModule(camera, m_pDisplay, &field);
	//AutoCalibrator visionModule(camera, this);
	MouseVision mouseVision(camera, m_pDisplay, &field);

	AutoCalibrator calibrator(camera, m_pDisplay);

	DistanceCalibrator distanceCalibrator(camera, m_pDisplay);

	/* Communication modules */
	ComModule comModule(wheels, coilBoard);

	/* Logic modules */
	NewAutoPilot autoPilot(&comModule, &field);

	ManualControl manualControl(&comModule);
	RemoteControl remoteControl(io, &comModule);

	//RobotTracker tracker(wheels);

	std::stringstream subtitles;

	VideoRecorder videoRecorder("videos/", 30, camera->GetFrameSize(true));
	while (true)
    {
		time = boost::posix_time::microsec_clock::local_time();
//		boost::posix_time::time_duration::tick_type dt = (time - lastStepTime).total_milliseconds();
		boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();
		/*
		if (dt > 1000) {
			fps = 1000.0 * frames / dt;
			lastStepTime = time;
			frames = 0;
		}
		*/
#define RECORD_AFTER_PROCESSING
#ifdef RECORD_AFTER_PROCESSING
		if (captureFrames) {
			videoRecorder.RecordFrame(camera->GetLastFrame(true), subtitles.str());
		}
#endif
		
#ifndef RECORD_AFTER_PROCESSING
		if (captureFrames) {
			videoRecorder.RecordFrame(cam1_area, subtitles.str());
		}
#endif


		std::ostringstream oss;
		oss.precision(4);

		oss << "[Robot] State: " << STATE_LABELS[state];
		/*
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
*/
		/*

		/* Main UI */
		if (STATE_NONE == state) {
			START_DIALOG
				mouseVision.Enable(false);
				calibrator.Enable(false);
				visionModule.Enable(true);
				autoPilot.Enable(false);
				manualControl.Enable(false);
				remoteControl.Enable(false);
				autoPilot.enableTestMode(false);
				distanceCalibrator.removeListener();
				wheels->Stop();
				STATE_BUTTON("(A)utoCalibrate objects", 'a', STATE_AUTOCALIBRATE)
				//STATE_BUTTON("(M)anualCalibrate objects", STATE_CALIBRATE)
				STATE_BUTTON("(C)Change Gate [" + OBJECT_LABELS[targetGate] + "]", 'c', STATE_SELECT_GATE)
				STATE_BUTTON("Auto(P)ilot [" + (autoPilot.running ? "On" : "Off") + "]", 'p', STATE_LAUNCH)
				/*
			createButton(std::string("(M)ouse control [") + (mouseControl == 0 ? "Off" : (mouseControl == 1 ? "Ball" : "Gate")) + "]", [this, &mouseControl]{
				mouseControl = (mouseControl + 1) % 3;
				this->last_state = STATE_END_OF_GAME; // force dialog redraw
			});
			*/
			//				STATE_BUTTON("(D)ance", STATE_DANCE)
				//STATE_BUTTON("(D)ance", STATE_DANCE)
				STATE_BUTTON("(R)emote Control", 'r', STATE_REMOTE_CONTROL)
				m_pDisplay->createButton(std::string("Save video: ") + (captureFrames ? "on" : "off"), 'v', [this, &captureDir, &time, &videoRecorder]{
					if (this->captureFrames) {
						// save old video
					}

					this->captureFrames = !this->captureFrames;
					this->captureFrames ? videoRecorder.Start() : videoRecorder.Stop();

					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				STATE_BUTTON("(S)ettings", 's', STATE_SETTINGS)
					m_pDisplay->createButton("Reinit wheels", '-', [this] {
					initPorts();
					initWheels();
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				m_pDisplay->createButton("Reinit coilboard", '-', [this] {
					initPorts();
					initCoilboard();
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				/*
				createButton("Swap displays", '-', [this] {
					m_bCam1Active = !m_bCam1Active;
				});
				createButton("Toggle main display on/off", '-', [this] {
					m_bMainCamEnabled = !m_bMainCamEnabled;
				});
				*/


				STATE_BUTTON("(M)anual Control", 'm', STATE_MANUAL_CONTROL)
				STATE_BUTTON("M(o)use vision", 'o', STATE_MOUSE_VISION)
				STATE_BUTTON("Test CoilGun", '-', STATE_TEST_COILGUN)
				STATE_BUTTON("(D)istance calibration", 'd', STATE_DISTANCE_CALIBRATE)
				STATE_BUTTON("Test Autopilot", '-', STATE_TEST)
				STATE_BUTTON("E(x)it", 27, STATE_END_OF_GAME)
			END_DIALOG
		}
		
		else if (STATE_AUTOCALIBRATE == state) {
			START_DIALOG
				visionModule.Enable(false);
				mouseVision.Enable(false);
				calibrator.Enable(true);
				calibrator.reset();
				m_pDisplay->createButton("Take a screenshot", '-', [this, &calibrator]{
					if (calibrator.LoadFrame()) {
						this->SetState(STATE_CALIBRATE);
					};

				});
			STATE_BUTTON("BACK", 8,STATE_NONE)
			END_DIALOG
		}
		else if (STATE_MOUSE_VISION == state) {
			START_DIALOG
				visionModule.Enable(false);
				calibrator.Enable(false);
				mouseVision.Enable(true);
			STATE_BUTTON("BACK", 8, STATE_NONE)
				END_DIALOG
		}

		else if (STATE_CALIBRATE == state) {
			START_DIALOG
				for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
					// objectThresholds[(OBJECT) i] = calibrator->GetObjectThresholds(i, OBJECT_LABELS[(OBJECT) i]);
				m_pDisplay->createButton(OBJECT_LABELS[(OBJECT)i], '-', [this, i, &calibrator]{
						/*this->objectThresholds[(OBJECT)i] =*/ calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					});
				}
				STATE_BUTTON("BACK", 8, STATE_NONE)
			END_DIALOG
		}
		else if (state == STATE_DISTANCE_CALIBRATE){			
			START_DIALOG
				distanceCalibrator.start();
				STATE_BUTTON("BACK", 8, STATE_NONE)
			END_DIALOG 
			m_pDisplay->putText("Last calibrated distance:", cv::Point(-250, 220), 0.5, cv::Scalar(255, 255, 255));
			m_pDisplay->putText(distanceCalibrator.counterValue, cv::Point(-250, 240), 0.5, cv::Scalar(255, 255, 255));
		}else if (STATE_SELECT_GATE == state) {
			START_DIALOG
				m_pDisplay->createButton(OBJECT_LABELS[BLUE_GATE], '-', [&field, this]{
				field.SetTargetGate(BLUE_GATE);
				this->SetState(STATE_NONE);
			});
			m_pDisplay->createButton(OBJECT_LABELS[YELLOW_GATE], '-', [&field, this]{
				field.SetTargetGate(YELLOW_GATE);
				this->SetState(STATE_NONE);
			});
			END_DIALOG
		}
		else if (STATE_SETTINGS == state) {
			START_DIALOG
				IConfigurableModule *pModule = static_cast<IConfigurableModule*>(&visionModule);
				for (auto setting : pModule->GetSettings()){
					m_pDisplay->createButton(setting.first + ": " + std::get<0>(setting.second)(), '-', [this, setting]{
						std::get<1>(setting.second)();
						this->last_state = STATE_END_OF_GAME; // force dialog redraw
					});
				}
				STATE_BUTTON("BACK", 8, STATE_NONE)
			END_DIALOG
		}
		else if (STATE_MANUAL_CONTROL == state) {
			START_DIALOG
				IConfigurableModule *pModule = static_cast<IConfigurableModule*>(&manualControl);
			for (auto setting : pModule->GetSettings()){
				m_pDisplay->createButton(setting.first + ": " + std::get<0>(setting.second)(), std::get<0>(setting.second)()[0], [this, setting]{
					std::get<1>(setting.second)();
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
			}
			STATE_BUTTON("BACK", 8, STATE_NONE)
			END_DIALOG
		}
		else if (STATE_REMOTE_CONTROL == state) {
			START_DIALOG;
			remoteControl.Enable(true);
			STATE_BUTTON("BACK", 8, STATE_NONE);
			END_DIALOG
		}
		else if (STATE_LAUNCH == state) {
			if (targetGate == NUMBER_OF_OBJECTS) {
				std::cout << "Select target gate" << std::endl;
				SetState(STATE_SELECT_GATE);
			}
			else {
				try {
					/*
					CalibrationConfReader calibrator;
					for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
						objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					}
					*/
					//SetState(STATE_SELECT_GATE);
					coilBoard->ToggleTribbler(false);
					wheels->Stop();
					autoPilot.Enable(!autoPilot.running);
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
				m_pDisplay->createButton(std::string("Save video: ") + (captureFrames ? "on" : "off"), '-', [this, &captureDir, &time, &videoRecorder]{
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
				STATE_BUTTON("(B)ack", 8, STATE_NONE)
				STATE_BUTTON("E(x)it", 27, STATE_END_OF_GAME)
			END_DIALOG

				
			
//			autoPilot->UpdateState(ballFound ? &ballPos : NULL, targetGatePos, sightObstructed);
			
		}
		else if (STATE_TEST == state) {
			START_DIALOG
				autoPilot.enableTestMode(true);
				for (const auto d : autoPilot.driveModes) {
					m_pDisplay->createButton(d.second->name, '-', [this, &autoPilot, d]{
						autoPilot.setTestMode(d.first);
					});
				}
				STATE_BUTTON("BACK", 8, STATE_NONE)
			END_DIALOG
		}
		else if (STATE_DANCE == state) {
			float move1, move2;
			dance_step(((float)(time - epoch).total_milliseconds()), move1, move2);
			wheels->Drive(move1, move2,0);
			//cv::putText(frameBGR, "move1:" + std::to_string(move1), cv::Point(frameBGR.cols - 140, 120), 0.5, cv::Scalar(255, 255, 255));
			//cv::putText(frameBGR, "move2:" + std::to_string(move2), cv::Point(frameBGR.cols - 140, 140), 0.5, cv::Scalar(255, 255, 255));
		}
		else if (STATE_END_OF_GAME == state) {
			break;
		}
		 
		subtitles.str("");
		//subtitles << oss.str();
		subtitles << "|" << autoPilot.GetDebugInfo();
		subtitles << "|" << wheels->GetDebugInfo();
		if (scanner && scanner->running) {
			subtitles << "|" << "Please wait, Scanning Ports";
		}
		else {
			if (!wheels->IsReal()) {
				subtitles << "|" << "WARNING: real wheels not connected!";
			}
			if (!coilBoardPortsOk) {
				subtitles << "   " << "WARNING: coilgun not connected!";
			}
		} 

		m_pDisplay->putText( "fps: " + std::to_string(camera->GetFPS()), cv::Point(-140, 20), 0.5, cv::Scalar(255, 255, 255));
		//assert(STATE_END_OF_GAME != state);
		m_pDisplay->putText( "state: " + STATE_LABELS[state], cv::Point(-140, 40), 0.5, cv::Scalar(255, 255, 255));
		ObjectPosition ballPos = field.balls[0];
		ObjectPosition targetGatePos = field.GetTargetGate();
		m_pDisplay->putText( std::string("Ball:") + (ballPos.getDistance() > 0 ? "yes" : "no"), cv::Point(-140, 60), 0.5, cv::Scalar(255, 255, 255));
		m_pDisplay->putText( std::string("Gate:") + (targetGatePos.getDistance() >0 ? "yes" : "no"), cv::Point(-140, 80), 0.5, cv::Scalar(255, 255, 255));
		
		m_pDisplay->putText( std::string("Trib:") + (coilBoard->BallInTribbler() ? "yes" : "no"), cv::Point(-140, 100), 0.5, cv::Scalar(255, 255, 255));
		m_pDisplay->putText( std::string("Sight:") + (field.gateObstructed ? "obst" : "free"), cv::Point(-140, 120), 0.5, cv::Scalar(255, 255, 255));
		//m_pDisplay->putText( std::string("OnWay:") + (somethingOnWay ? "yes" : "no"), cv::Point(-140, 140), 0.5, cv::Scalar(255, 255, 255));
		
		for (int i = 0; i < NUMBER_OF_BALLS; i++) {
			BallPosition ball = field.balls[i].load();
			m_pDisplay->putText( std::string("Ball") + std::to_string(i) + ": "+ std::to_string(ball.fieldCoords.x) + " : " + std::to_string(ball.fieldCoords.y), cv::Point(-250, i * 15 + 10), 0.5, cv::Scalar(255, 255, 255));
		}

		m_pDisplay->putText( "robot: " + std::to_string(field.self.load().fieldCoords.x) + " " + std::to_string(field.self.load().fieldCoords.y), cv::Point(-140, 200), 0.5, cv::Scalar(255, 255, 255));


		//m_pDisplay->putText( "border: " + std::to_string(borderDistance.distance), cv::Point(-140, 280), 0.5, cv::Scalar(255, 255, 255));


		m_pDisplay->putText( "Yellow Gate" ,  cv::Point(-140, 320), 0.5, cv::Scalar(255, 255, 255));
		m_pDisplay->putText( "(dist / angle): " + std::to_string(field.yellowGate.load().getDistance()) + " / " + std::to_string(field.yellowGate.load().getAngle()), cv::Point(-140, 340), 0.5, cv::Scalar(255, 255, 255));
		

		
		//TODO: fix putText newline thing
		std::vector<std::string> subtitles2;
		std::string subtitles3 = subtitles.str();

		boost::split(subtitles2, subtitles3, boost::is_any_of("|"));

		int j = 0;
		for (auto s : subtitles2) {
			if (s.empty()) s = " "; // opencv 3 crashes on empty string
			m_pDisplay->putText( s, cv::Point(10, -150 + j), 0.5, cv::Scalar(255, 255, 255));
			j += 20;
		}

		/* robot tracker */
		cv::Point2i center(-100, 200);
		double velocity = 0, direction = 0, rotate = 0;
		auto speed = wheels->GetTargetSpeed();

		/*
		//Draw circle
		cv::Scalar colorCircle(133, 33, 55);
		cv::circle(display, center, 60, colorCircle, 2);
		*/
		m_pDisplay->Draw();
		/*
		int key = cv::waitKey(1);
		if (key != -1) {
			KeyPressed(key);
		}
		if (key == 27) {
			std::cout << "exiting program" << std::endl;
			break;
		}
		*/
//		frames++;

    }
    	
	if (outputVideo != NULL) {
		delete outputVideo;
	}


}


bool Robot::ParseOptions(int argc, char* argv[])
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("camera", po::value<std::string>(), "set camera index or path")
		("locate_cursor", "find cursor instead of ball")
		("skip-ports", "skip ALL COM port checks")
		("skip-missing-ports", "skip missing COM ports")
		("save-frames", "Save captured frames to disc");

	po::store(po::parse_command_line(argc, argv, desc), config);
	po::notify(config);

	if (config.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	return true;
}
