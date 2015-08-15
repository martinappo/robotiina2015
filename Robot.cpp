#include "Robot.h"
#include "AutoCalibrator.h"

#include "Camera.h"
#include "StillCamera.h"
#include "WheelController.h"
#include "CoilBoard.h"
#include "Dialog.h"
#include "Wheel.h"
#include "ComPortScanner.h"

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
#include "NewAutoPilot.h"
#include "RobotTracker.h"
#include "VideoRecorder.h"
#include "FrontCameraVision.h"
#include "ComModule.h"
#include "ManualControl.h"
#include "SoccerField.h"
#include "MouseVision.h"

#define STATE_BUTTON(name, shortcut, new_state) \
createButton(std::string("") + name, shortcut, [&](){ this->SetState(new_state); });
#define BUTTON(name, shortcut, function_body) \
createButton(name, shortcut, [&]() function_body);
#define START_DIALOG if (state != last_state) { \
clearButtons();
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
	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};

std::map<STATE, std::string> STATE_LABELS(states, states + sizeof(states) / sizeof(states[0]));

/* BEGIN DANCE MOVES */
void dance_step(float time, float &move1, float &move2) {
	move1 = 50*sin(time/4000);
	move2 = 360 * cos(time / 4000);
}

/* END DANCE MOVES */

Robot::Robot(boost::asio::io_service &io) : Dialog("Robotiina"), io(io), camera(0), wheels(0), coilBoard(0)
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
}

bool Robot::Launch(int argc, char* argv[])
{
	if (!ParseOptions(argc, argv)) return false;

	// Compose robot from its parts

	wheels = new WheelController();
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
		std::cout << "Checking COM ports... " << std::endl;
		ComPortScanner scanner;
		if ((scanner.VerifyAll(io)) == false) {
			std::cout << "Ports check failed, rescanning all ports" << std::endl;
			scanner.Scan(io);
		}
		wheelsPortsOk = scanner.VerifyWheels(io);
		coilBoardPortsOk = scanner.VerifyCoilboard(io);
	}
}

void Robot::initWheels()
{
	if (wheelsPortsOk) {
		std::cout << "Using real wheels" << std::endl;
		try {
			wheels->InitWheels(io);
		}
		catch (...) {
			throw;
		}
	}
	else {
		std::cout << "WARNING: Using dummy wheels" << std::endl;
		wheels->InitDummyWheels();
	}
	
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
	SoccerField field(this);

	/* Vision modules */
	FrontCameraVision visionModule(camera, this, &field);
	MouseVision mouseVision(camera, this, &field);

	AutoCalibrator calibrator(camera, this);

	/* Communication modules */
	ComModule comModule(wheels, coilBoard);

	/* Logic modules */
	NewAutoPilot autoPilot(&comModule, &field);

	ManualControl manualControl(&comModule);

	//RobotTracker tracker(wheels);


	
	std::stringstream subtitles;



	VideoRecorder videoRecorder("videos/", 30, display.size());

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
			videoRecorder.RecordFrame(display, subtitles.str());
		}
#endif
		
#ifndef RECORD_AFTER_PROCESSING
		if (captureFrames) {
			videoRecorder.RecordFrame(display, subtitles.str());
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
				autoPilot.enableTestMode(false);
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
				//STATE_BUTTON("(R)emote Control", STATE_REMOTE_CONTROL)
				createButton(std::string("Save video: ") + (captureFrames ? "on" : "off"), 'v', [this, &captureDir, &time, &videoRecorder]{
					if (this->captureFrames) {
						// save old video
					}

					this->captureFrames = !this->captureFrames;
					this->captureFrames ? videoRecorder.Start() : videoRecorder.Stop();

					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				STATE_BUTTON("(S)ettings", 's', STATE_SETTINGS)
				createButton("Reinit wheels", '-', [this] {
					initPorts();
					initWheels();
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				createButton("Reinit coilboard", '-', [this] {
					initPorts();
					initCoilboard();
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				createButton("Swap displays", '-', [this] {
					m_bCam1Active = !m_bCam1Active;
				});
				createButton("Toggle main display on/off", '-', [this] {
					m_bMainCamEnabled = !m_bMainCamEnabled;
				});


				STATE_BUTTON("(M)anual Control", 'm', STATE_MANUAL_CONTROL)
				STATE_BUTTON("M(o)use vision", 'o', STATE_MOUSE_VISION)
				STATE_BUTTON("Test CoilGun", '-', STATE_TEST_COILGUN)
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
				createButton("Take a screenshot", '-',[this, &calibrator]{
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
					createButton(OBJECT_LABELS[(OBJECT)i], '-', [this, i, &calibrator]{
						/*this->objectThresholds[(OBJECT)i] =*/ calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					});
				}
				STATE_BUTTON("BACK", 8, STATE_NONE)
			END_DIALOG
		}
		
		else if (STATE_SELECT_GATE == state) {
			START_DIALOG
				createButton(OBJECT_LABELS[BLUE_GATE], '-', [&field, this]{
				field.SetTargetGate(BLUE_GATE);
				this->SetState(STATE_NONE);
			});
			createButton(OBJECT_LABELS[YELLOW_GATE], '-', [&field, this]{
				field.SetTargetGate(YELLOW_GATE);
				this->SetState(STATE_NONE);
			});
			END_DIALOG
		}
		else if (STATE_SETTINGS == state) {
			START_DIALOG
				IConfigurableModule *pModule = static_cast<IConfigurableModule*>(&visionModule);
				for (auto setting : pModule->GetSettings()){
					createButton(setting.first + ": " + std::get<0>(setting.second)(), '-', [this, setting]{
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
					createButton(setting.first + ": " + std::get<0>(setting.second)(), std::get<0>(setting.second)()[0], [this, setting]{
						std::get<1>(setting.second)();
						this->last_state = STATE_END_OF_GAME; // force dialog redraw
					});
				}
				STATE_BUTTON("BACK", 8,STATE_NONE)
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
				createButton(std::string("Save video: ") + (captureFrames ? "on" : "off"), '-', [this, &captureDir, &time, &videoRecorder]{
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
					createButton(d.second->name, '-',[this, &autoPilot, d]{
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
			//cv::putText(frameBGR, "move1:" + std::to_string(move1), cv::Point(frameBGR.cols - 140, 120), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			//cv::putText(frameBGR, "move2:" + std::to_string(move2), cv::Point(frameBGR.cols - 140, 140), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		}
		else if (STATE_END_OF_GAME == state) {
			break;
		}
		 
		subtitles.str("");
		//subtitles << oss.str();
		subtitles << "|" << autoPilot.GetDebugInfo();
		subtitles << "|" << wheels->GetDebugInfo();
		if (!wheelsPortsOk) {
			subtitles << "|" << "WARNING: real wheels not connected!";
		}
		if (!coilBoardPortsOk) {
			subtitles << "   " << "WARNING: coilgun not connected!";
		}

		cv::putText(display, "fps: " + std::to_string(camera->GetFPS()), cv::Point(display.cols - 140, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		//assert(STATE_END_OF_GAME != state);
		cv::putText(display, "state: " + STATE_LABELS[state], cv::Point(display.cols - 140, 40), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		ObjectPosition ballPos = field.balls[0];
		ObjectPosition targetGatePos = field.GetTargetGate();
		cv::putText(display, std::string("Ball:") + (ballPos.getDistance() > 0 ? "yes" : "no"), cv::Point(display.cols - 140, 60), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, std::string("Gate:") + (targetGatePos.getDistance() >0 ? "yes" : "no"), cv::Point(display.cols - 140, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		
		cv::putText(display, std::string("Trib:") + (coilBoard->BallInTribbler() ? "yes" : "no"), cv::Point(display.cols - 140, 100), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, std::string("Sight:") + (field.gateObstructed ? "obst" : "free"), cv::Point(display.cols - 140, 120), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		//cv::putText(display, std::string("OnWay:") + (somethingOnWay ? "yes" : "no"), cv::Point(display.cols - 140, 140), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		
		cv::putText(display, "Ball" , cv::Point(display.cols - 140, 180), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, "dist: " + std::to_string(ballPos.getDistance()), cv::Point(display.cols - 140, 200), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(display, "angle :" + std::to_string(ballPos.getAngle()), cv::Point(display.cols - 140, 220), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		//cv::putText(display, "border: " + std::to_string(borderDistance.distance), cv::Point(display.cols - 140, 280), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		if (targetGatePos.getDistance() > 0) {
			cv::putText(display, "Gate" ,  cv::Point(display.cols - 140, 320), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			cv::putText(display, "dist: " + std::to_string(targetGatePos.getDistance()), cv::Point(display.cols - 140, 340), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			cv::putText(display, "angle: " + std::to_string(targetGatePos.getAngle()), cv::Point(display.cols - 140, 360), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
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
			if (s.empty()) s = " "; // opencv 3 crashes on empty string
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
		int key = cv::waitKey(1);
		if (key != -1) {
			KeyPressed(key);
		}
		if (key == 27) {
			std::cout << "exiting program" << std::endl;
			break;
		}
//		frames++;

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
            wheels->Drive(speed, direction,0);
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
