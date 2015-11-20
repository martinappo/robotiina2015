#include "Robot.h"
#include "AutoCalibrator.h"
#include "DistanceCalibrator.h"
#include "DistanceCalculator.h"
#include "Simulator.h"

#include "Camera.h"
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
#include "SingleModePlay.h"
#include "MultiModePlay.h"
#include "RobotTracker.h"
#include "VideoRecorder.h"
#include "FrontCameraVision.h"
#include "ComModule.h"
#include "ManualControl.h"
#ifdef ENABLE_REMOTE_CONTROL
#include "RemoteControl.h"
#endif
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
	std::pair<STATE, std::string>(STATE_GIVE_COMMAND, "Give Referee Command"),
	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};

std::pair<std::string, FieldState::GameMode> refCommands[] = {
	std::pair<std::string, FieldState::GameMode>("Start game", FieldState::GAME_MODE_START_SINGLE_PLAY),
	std::pair<std::string, FieldState::GameMode>("Stop game", FieldState::GAME_MODE_STOPED),
	std::pair<std::string, FieldState::GameMode>("Placed ball", FieldState::GAME_MODE_PLACED_BALL),
	std::pair<std::string, FieldState::GameMode>("End half", FieldState::GAME_MODE_END_HALF),

	std::pair<std::string, FieldState::GameMode>("Our kickoff", FieldState::GAME_MODE_START_OUR_KICK_OFF),
	std::pair<std::string, FieldState::GameMode>("Our indirect free kick", FieldState::GAME_MODE_START_OUR_INDIRECT_FREE_KICK),
	std::pair<std::string, FieldState::GameMode>("Our direct free kick", FieldState::GAME_MODE_START_OUR_FREE_KICK),
	std::pair<std::string, FieldState::GameMode>("Our goal kick", FieldState::GAME_MODE_START_OUR_GOAL_KICK),
	std::pair<std::string, FieldState::GameMode>("Our throw in", FieldState::GAME_MODE_START_OUR_THROWIN),
	std::pair<std::string, FieldState::GameMode>("Our corner kick", FieldState::GAME_MODE_START_OUR_CORNER_KICK),
	std::pair<std::string, FieldState::GameMode>("Our penalty", FieldState::GAME_MODE_START_OUR_PENALTY),
	std::pair<std::string, FieldState::GameMode>("Our goal", FieldState::GAME_MODE_START_OUR_GOAL),
	std::pair<std::string, FieldState::GameMode>("Our yellow card", FieldState::GAME_MODE_START_OUR_YELLOW_CARD),
/*
	std::pair<std::string, FieldState::GameMode>("Opponent kickoff", FieldState::GAME_MODE_START_OPPONENT_KICK_OFF),
	std::pair<std::string, FieldState::GameMode>("Opponent indirect free kick", FieldState::GAME_MODE_START_OPPONENT_INDIRECT_FREE_KICK),
	std::pair<std::string, FieldState::GameMode>("Opponent direct free kick", FieldState::GAME_MODE_START_OPPONENT_FREE_KICK),
	std::pair<std::string, FieldState::GameMode>("Opponent goal kick", FieldState::GAME_MODE_START_OPPONENT_GOAL_KICK),
	std::pair<std::string, FieldState::GameMode>("Opponent throw in", FieldState::GAME_MODE_START_OPPONENT_THROWIN),
	std::pair<std::string, FieldState::GameMode>("Opponent corner kick", FieldState::GAME_MODE_START_OPPONENT_CORNER_KICK),
	std::pair<std::string, FieldState::GameMode>("Opponent penalty", FieldState::GAME_MODE_START_OPPONENT_PENALTY),
	std::pair<std::string, FieldState::GameMode>("Opponent goal", FieldState::GAME_MODE_START_OPPONENT_GOAL),
	std::pair<std::string, FieldState::GameMode>("Opponent yellow card", FieldState::GAME_MODE_START_OPPONENT_YELLOW_CARD)
*/
};

DistanceCalculator gDistanceCalculator;
//TODO: convert to commandline options
//#define USE_ROBOTIINA_WIFI
#ifdef USE_ROBOTIINA_WIFI 
// robotiina wifi
boost::asio::ip::address bind_addr = boost::asio::ip::address::from_string("10.0.0.6"); // this computer ip
boost::asio::ip::address brdc_addr = boost::asio::ip::address::from_string("10.0.0.15"); // netmask 255.255.255.240
#else
// any local network
boost::asio::ip::address bind_addr = boost::asio::ip::address::from_string("0.0.0.0"); // all interfaces
boost::asio::ip::address brdc_addr = boost::asio::ip::address_v4::broadcast(); // local network
#endif

std::map<STATE, std::string> STATE_LABELS(states, states + sizeof(states) / sizeof(states[0]));

/* BEGIN DANCE MOVES */
void dance_step(float time, float &move1, float &move2) {
	move1 = 50*sin(time/4000);
	move2 = 360 * cos(time / 4000);
}

/* END DANCE MOVES */

Robot::Robot(boost::asio::io_service &io) : io(io), m_pCamera(0)
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
	Simulator * pSim = dynamic_cast<Simulator*>(m_pCamera);
	if (pSim != NULL) {
		delete pSim;
		m_pCamera = NULL;
		m_pSerial = NULL;
		refCom = NULL;
	}
	if (m_pCamera) {
		delete m_pCamera;
	}
	if (refCom)
		delete refCom;
	if (m_pSerial)
		delete m_pSerial;

}


bool Robot::Launch(int argc, char* argv[])
{
	if (!ParseOptions(argc, argv)) return false;
	if (config.count("play-mode"))
		play_mode = config["play-mode"].as<std::string>();

	std::string _cam = config.count("camera") >0? config["camera"].as<std::string>():"";
	bool bSimulator = _cam == "simulator" || _cam == "simulator-master";
	if (bSimulator) {
		InitSimulator(_cam == "simulator-master", play_mode);
	}
	else {
		InitHardware();
	}
	std::cout << "Starting Robot" << std::endl;

	cv::Size winSize(0, 0);
	if (config.count("app-size")) {
		std::vector<std::string> tokens;
		boost::split(tokens, config["app-size"].as<std::string>(), boost::is_any_of("x"));
		winSize.width = atoi(tokens[0].c_str());
		winSize.height = atoi(tokens[1].c_str());

	}
	// Compose robot from its parts
	if (config.count("webui") == 0)
		m_pDisplay = new Dialog("Robotiina", winSize, m_pCamera->GetFrameSize());
	else
		m_pDisplay = new WebUI(8080);
	captureFrames = config.count("capture-frames") > 0;

	Run();

	return true;
}
void Robot::InitSimulator(bool master, const std::string game_mode) {
	auto pSim = new Simulator(io, master, game_mode);
	m_pCamera = pSim;
	m_pSerial = pSim;
	refCom = pSim;
}

void Robot::InitHardware() {
	std::cout << "Initializing Camera... " << std::endl;
	if (config.count("camera"))
		if (config["camera"].as<std::string>() == "ximea")
			m_pCamera = new Camera(CV_CAP_XIAPI);
		else
			m_pCamera = new Camera(config["camera"].as<std::string>());
	else
		m_pCamera = new Camera(0);
	std::cout << "Done" << std::endl;
	initRefCom();
	try {
		using boost::property_tree::ptree;
		ptree pt;
		read_ini("conf/ports.ini", pt);
		std::string port = pt.get<std::string>(std::to_string(ID_COM));
		m_pSerial = new SimpleSerial(io, port, 19200);
		//Sleep(100);
	}
	catch (std::exception const&  ex) {
		std::cout << "Main board com port fail: " << ex.what() << std::endl;
	}
	
	std::cout << "Done initializing" << std::endl;
	return;
}

void Robot::initRefCom() {
	std::cout << "Init referee communications" << std::endl;
	try {
		using boost::property_tree::ptree;
		ptree pt;
		read_ini("conf/ports.ini", pt);
		std::string port = pt.get<std::string>(std::to_string(ID_REF));
		refCom = new LLAPReceiver(NULL, io, port);
	}
	catch (...) { 
		std::cout << "Referee com LLAP reciever couldn't be initialized" << std::endl;
		refCom = new RefereeCom(NULL);
	}
}
/*
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
*/
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

	SoccerField field(io, m_pDisplay, play_mode == "master" || play_mode == "single", play_mode == "master" || play_mode == "slave" ? 1 : 11);
	refCom->setField(&field);

	/* Vision modules */
	FrontCameraVision visionModule(m_pCamera, m_pDisplay, &field);
	//AutoCalibrator visionModule(m_pCamera, this);
	MouseVision mouseVision(m_pCamera, m_pDisplay, &field);

	AutoCalibrator calibrator(m_pCamera, m_pDisplay);

	DistanceCalibrator distanceCalibrator(m_pCamera, m_pDisplay);

	/* Communication modules */
	ComModule comModule(m_pSerial);

	/* Logic modules */
	StateMachine *autoPilot = NULL;
	if (play_mode=="master" || play_mode =="slave"){
		autoPilot = new MultiModePlay(&comModule, &field, play_mode == "master");
	}
	else  {
		autoPilot = new SingleModePlay(&comModule, &field);
	}

	ManualControl manualControl(&comModule);
#ifdef ENABLE_REMOTE_CONTROL
	RemoteControl remoteControl(io, &comModule);
#endif

	//RobotTracker tracker(wheels);

	std::stringstream subtitles;

	VideoRecorder videoRecorder("videos/", 30, m_pCamera->GetFrameSize(true));
	//port.get_io_service().run();
	while (true)
    {
		//io.poll_one();
		time = boost::posix_time::microsec_clock::local_time();
//		boost::posix_time::time_duration::tick_type dt = (time - lastStepTime).total_milliseconds();
		boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();

		/*
		auto &ballPos = field.balls[0];
		auto &targetGatePos = field.GetTargetGate();

		autoPilot->UpdateState(&ballPos, &targetGatePos);
		*/
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
			videoRecorder.RecordFrame(m_pCamera->GetLastFrame(true), subtitles.str());
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

		/* Main UI */
		if (STATE_NONE == state) {
			START_DIALOG
				mouseVision.Enable(false);
				calibrator.Enable(false);
				visionModule.Enable(true);
				if (last_state == STATE_TEST){
					autoPilot->Enable(false);
				}
				manualControl.Enable(false);
#ifdef ENABLE_REMOTE_CONTROL
				remoteControl.Enable(false);
#endif
				autoPilot->enableTestMode(false);
				distanceCalibrator.Enable(false);
				comModule.Drive(0);
				STATE_BUTTON("(A)utoCalibrate objects", 'a', STATE_AUTOCALIBRATE)
				//STATE_BUTTON("(M)anualCalibrate objects", STATE_CALIBRATE)
				STATE_BUTTON("(C)Change Gate [" + ((int)field.GetTargetGate().getDistance() == (int)(field.blueGate.getDistance()) ? "blue" : "yellow") + "]", 'c', STATE_SELECT_GATE)
				STATE_BUTTON("(G)ive Referee Command", 'g', STATE_GIVE_COMMAND)
				STATE_BUTTON("Auto(P)ilot [" + (autoPilot->running ? "On" : "Off") + "]", 'p', STATE_LAUNCH)

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
				std::stringstream sset;
				sset << " [ robot: " << refCom->FIELD_MARKER << refCom->ROBOT_MARKER << ", team: " << refCom->TEAM_MARKER << "]";

				STATE_BUTTON("(S)ettings" + sset.str(), 's', STATE_SETTINGS)
					m_pDisplay->createButton("Reinit wheels", '-', [this] {
					//initPorts();
					//initWheels();
					//TODO: fix this
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				m_pDisplay->createButton("Reinit coilboard", '-', [this] {
					//initPorts();
					//initCoilboard();
					this->last_state = STATE_END_OF_GAME; // force dialog redraw
				});
				
				m_pDisplay->createButton("Swap displays", '-', [this] {
					m_pDisplay->SwapDisplays();
				});
				m_pDisplay->createButton("Toggle main display on/off", '-', [this] {
					m_pDisplay->ToggleDisplay();
				});
				m_pDisplay->createButton("Pause/Play video", 'f', [this] {
					m_pCamera->TogglePlay();
				});



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
					calibrator.LoadFrame();
					this->SetState(STATE_CALIBRATE);
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
				distanceCalibrator.Enable(true);
				STATE_BUTTON("BACK", 8, STATE_NONE)
			END_DIALOG 
			m_pDisplay->putText(distanceCalibrator.message, cv::Point(250, 220), 0.5, cv::Scalar(0, 0, 255));
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
				pModule = static_cast<IConfigurableModule*>(refCom);
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
			manualControl.Enable(true);
			END_DIALOG
		}
#ifdef ENABLE_REMOTE_CONTROL
		else if (STATE_REMOTE_CONTROL == state) {
			START_DIALOG;
			remoteControl.Enable(true);
			STATE_BUTTON("BACK", 8, STATE_NONE);
			END_DIALOG
		}
#endif
		else if (STATE_LAUNCH == state) {
			//if (targetGate == NUMBER_OF_OBJECTS) {
			//	std::cout << "Select target gate" << std::endl;
			//	SetState(STATE_SELECT_GATE);
			//}
			//else {
				try {
					/*
					CalibrationConfReader calibrator;
					for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
						objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					}
					*/
					//SetState(STATE_SELECT_GATE);
					comModule.ToggleTribbler(0);
					comModule.Drive(0);
					autoPilot->Enable(!autoPilot->running);
					SetState(STATE_NONE);
				}
				catch (...){
					std::cout << "Calibration data is missing!" << std::endl;
					//TODO: display error
					SetState(STATE_AUTOCALIBRATE); // no conf
				}
			//}
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
			
			
		}
		else if (STATE_TEST == state) {
			START_DIALOG
				autoPilot->enableTestMode(true);
				autoPilot->Enable(true);
				for (const auto d : autoPilot->driveModes) {
					m_pDisplay->createButton(d.second->name, '-', [this, &autoPilot, d]{
						autoPilot->setTestMode(d.first);
					});
				}
				last_state = STATE_TEST;
				STATE_BUTTON("BACK", 8, STATE_NONE)
			END_DIALOG
		}
		else if (STATE_DANCE == state) {
			float move1, move2;
			dance_step(((float)(time - epoch).total_milliseconds()), move1, move2);
			comModule.Drive(move1, move2,0);
			//cv::putText(frameBGR, "move1:" + std::to_string(move1), cv::Point(frameBGR.cols - 140, 120), 0.5, cv::Scalar(255, 255, 255));
			//cv::putText(frameBGR, "move2:" + std::to_string(move2), cv::Point(frameBGR.cols - 140, 140), 0.5, cv::Scalar(255, 255, 255));
		}
		else if (STATE_GIVE_COMMAND == state) {
			START_DIALOG
				for (std::pair < std::string, FieldState::GameMode> entry : refCommands) {
					m_pDisplay->createButton(entry.first, '-', [this, entry]{
						refCom->giveCommand(entry.second);
					});
				}
			STATE_BUTTON("BACK", 8, STATE_NONE)
				END_DIALOG
		}
		else if (STATE_END_OF_GAME == state) {
			break;
		}
		 
		subtitles.str("");
		//subtitles << oss.str();
		subtitles << "|" << autoPilot->GetDebugInfo();
		subtitles << "|" << comModule.GetDebugInfo();
		if (!comModule.IsReal()) {
			subtitles << "|" << "WARNING: Serial not connected!";
		}

		m_pDisplay->putText( "fps: " + std::to_string(m_pCamera->GetFPS()), cv::Point(-140, 20), 0.5, cv::Scalar(255, 255, 255));
		//assert(STATE_END_OF_GAME != state);
		m_pDisplay->putText( "state: " + STATE_LABELS[state], cv::Point(-140, 40), 0.5, cv::Scalar(255, 255, 255));
		//m_pDisplay->putText( std::string("Ball:") + (ballPos.getDistance() > 0 ? "yes" : "no"), cv::Point(-140, 60), 0.5, cv::Scalar(255, 255, 255));
		//m_pDisplay->putText( std::string("Gate:") + (targetGatePos.getDistance() >0 ? "yes" : "no"), cv::Point(-140, 80), 0.5, cv::Scalar(255, 255, 255));

		
//		m_pDisplay->putText( std::string("Trib:") + (coilBoard->BallInTribbler() ? "yes" : "no"), cv::Point(-140, 100), 0.5, cv::Scalar(255, 255, 255));
		m_pDisplay->putText( std::string("Sight:") + (field.gateObstructed ? "obst" : "free"), cv::Point(-140, 120), 0.5, cv::Scalar(255, 255, 255));
		//m_pDisplay->putText( std::string("OnWay:") + (somethingOnWay ? "yes" : "no"), cv::Point(-140, 140), 0.5, cv::Scalar(255, 255, 255));
		
		for (int i = 0; i < field.balls.size(); i++) {

			BallPosition &ball = field.balls[i];
			m_pDisplay->putText( std::string("Ball") + std::to_string(i) + ": "+ std::to_string(ball.polarMetricCoords.x) + " : " + std::to_string(ball.polarMetricCoords.y), cv::Point(-250, i * 15 + 10), 0.3, cv::Scalar(255, 255, 255));
		}

		m_pDisplay->putText("robot x:" + std::to_string(field.self.fieldCoords.x) + " y: " + std::to_string(field.self.fieldCoords.y) + " r: " + std::to_string(field.self.getAngle()), cv::Point(-250, 200), 0.4, cv::Scalar(255, 255, 255));
//		if (pSim != NULL)
//			m_pDisplay->putText("simul x:" + std::to_string(pSim->self.fieldCoords.x) + " y: " + std::to_string(pSim->self.fieldCoords.y) + " r: " + std::to_string(pSim->self.getAngle()), cv::Point(-250, 220), 0.4, cv::Scalar(255, 255, 255));



		//m_pDisplay->putText( "border: " + std::to_string(borderDistance.distance), cv::Point(-140, 280), 0.5, cv::Scalar(255, 255, 255));

		m_pDisplay->putText("Blue gate d: " + std::to_string((int)field.blueGate.getDistance()) + " a: " + std::to_string(field.blueGate.getAngle()), cv::Point(-250, 260), 0.4, cv::Scalar(255, 255, 255));
//		if (pSim != NULL)
//			m_pDisplay->putText("Blue gate d: " + std::to_string((int)pSim->blueGate.getDistance()) + " a: " + std::to_string(pSim->blueGate.getAngle()), cv::Point(-250, 280), 0.4, cv::Scalar(255, 255, 255));
		m_pDisplay->putText("Yell gate d: " + std::to_string((int)field.yellowGate.getDistance()) + " a: " + std::to_string(field.yellowGate.getAngle()), cv::Point(-250, 310), 0.4, cv::Scalar(255, 255, 255));
//		if (pSim != NULL)
//			m_pDisplay->putText("Yell gate d: " + std::to_string((int)pSim->yellowGate.getDistance()) + " a: " + std::to_string(pSim->yellowGate.getAngle()), cv::Point(-250, 330), 0.4, cv::Scalar(255, 255, 255));

		

		
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
		auto speed = comModule.GetTargetSpeed();

		/*
		//Draw circle
		cv::Scalar colorCircle(133, 33, 55);
		cv::circle(display, center, 60, colorCircle, 2);
		*/
		//m_pDisplay->Draw();
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

	if (autoPilot != NULL)
		delete autoPilot;
	refCom->setField(NULL);
}


bool Robot::ParseOptions(int argc, char* argv[])
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("camera", po::value<std::string>(), "set m_pCamera index or path")
		("app-size", po::value<std::string>(), "main window size: width x height")
		("locate_cursor", "find cursor instead of ball")
		("skip-ports", "skip ALL COM port checks")
		("skip-missing-ports", "skip missing COM ports")
		("save-frames", "Save captured frames to disc")
		("play-mode", po::value<std::string>(), "Play mode: single, opponent, master, slave")
		("twitter-port", po::value<int>(), "UDP port for communication between robots");


	po::store(po::parse_command_line(argc, argv, desc), config);
	po::notify(config);

	if (config.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	return true;
}
