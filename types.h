#pragma once
#pragma warning(disable: 4819)
#include <opencv2/opencv.hpp>
#include <math.h> 
#include <functional>
#include <atomic>
//#define VIRTUAL_FLIP
#ifdef WIN32
	#define _WIN32_WINNT 0x0600 // vista for socket.cancel()
/*
	#ifndef _WIN32_WINNT_WS08
		#define _WIN32_WINNT_WS08 // GetTickCount64 is missing in mingw, so emulate Windows Server 2008
	#endif
*/
#endif
#define PI 3.14159265
#define TAU (2*PI)

struct ColorRange
{
    int low;
    int high;
};

struct HSVColorRange
{
    ColorRange hue;
    ColorRange sat;
    ColorRange val;
};

struct Speed
{
	double velocity;
	double heading;
	double rotation;
};
//for conf file
const int ID_COM = 1;
const int ID_REF = 2;

//for actual ID info is sent to
const int ID_MAIN_BOARD = 5;

enum OBJECT
{
    BALL = 0, BLUE_GATE, YELLOW_GATE, FIELD, INNER_BORDER, OUTER_BORDER, NUMBER_OF_OBJECTS, SIGHT_MASK
};


enum STATE
{
    STATE_NONE = 0,
    STATE_AUTOCALIBRATE,
    STATE_CALIBRATE,
    STATE_LAUNCH,
	STATE_SELECT_GATE,
	STATE_RUN,
	STATE_SETTINGS,
	STATE_REMOTE_CONTROL,
	STATE_MANUAL_CONTROL,
	STATE_DANCE,
	STATE_TEST,
	STATE_TEST_COILGUN,
	STATE_MOUSE_VISION,
	STATE_DISTANCE_CALIBRATE,
	STATE_GIVE_COMMAND,
	STATE_END_OF_GAME /* leave this last*/
};
/*
enum REFCOMMAND
{
	START = 0,
	STOP,
	PLACED_BALL,
	END_HALF,
	KICKOFF,
	INDIRECT_FREE_KICK,
	DIRECT_FREE_KICK,
	GOAL_KICK,
	THROW_IN,
	CORNER_KICK,
	PENALTY,
	GOAL,
	YELLOW_CARD
}; */
class ISerialListener {
public:
	virtual void DataReceived(const std::string & message) = 0;
};
class ISerial : public ISerialListener{
public:
	virtual void SendCommand(int id, const std::string &cmd, int param = INT_MAX) = 0;
	virtual void WriteString(const std::string &s) = 0;
	virtual void SetMessageHandler(ISerialListener* callback) {};
};

class IObjectPosition {
public:
	virtual void updateRawCoordinates(const cv::Point pos, cv::Point orgin = cv::Point(0,0)) = 0; // Takes raw coordinates of object from frame
	virtual double getAngle() = 0;
	virtual double getDistance() = 0;
	virtual cv::Point getFieldPos() = 0;
};

class IUIEventListener {
public:
	// xy coordinates are from 0...1.0...
	virtual bool OnMouseEvent(int event, float x, float y, int flags, bool bMainArea) { return false; };
	virtual void OnKeyPress(char key) {};
};

class IDisplay {
public:
	virtual int createButton(const std::string& bar_name, char shortcut, std::function<void()> const &) = 0;
	virtual int Draw() = 0;
	virtual void clearButtons() = 0;
	virtual void ShowImage(const cv::Mat &image, bool main = true) = 0;
	virtual void AddEventListener(IUIEventListener *pEventListener) = 0;
	virtual void RemoveEventListener(IUIEventListener *pEventListener) = 0;
	virtual void putText(const std::string &text, cv::Point pos, double fontScale, cv::Scalar color) = 0;
	virtual void putShadowedText(const std::string &text, cv::Point pos, double fontScale, cv::Scalar color) = 0;
	virtual void SwapDisplays() = 0;
	virtual void ToggleDisplay() = 0;
	virtual ~IDisplay(){};
};

class ICamera
{
public:
	virtual cv::Mat & Capture(bool bFullFrame = false) = 0;
	virtual cv::Size GetFrameSize(bool bFullFrame = false) = 0;
	virtual double GetFPS() = 0;
	virtual cv::Mat & GetLastFrame(bool bFullFrame = false) = 0;
	virtual void TogglePlay() = 0;
};

//maybe use std::vector instead
typedef std::map<std::string, std::tuple<std::function<std::string()>, std::function<void()>>> SettingsList;

class IConfigurableModule {
public:
	virtual SettingsList &GetSettings() = 0;
	virtual std::string GetDebugInfo() = 0;
};

class IVisionModule {
public:
//	virtual bool Init(ICamera * pCamera, IDisplay *pDisplay, FieldState *pFieldState) = 0;
	virtual const cv::Mat & GetFrame() = 0;
};

/*
class IRefereeCom {
public:
	virtual bool isCommandAvailable() = 0;
	virtual REFCOMMAND getNextCommand() = 0;
	virtual void giveCommand(REFCOMMAND command) = 0;
	virtual bool isTogglable() = 0;
};*/

class ICommunicationModule {
public:
	virtual void Drive(double fowardSpeed, double direction = 0, double angularSpeed = 0) = 0;
	// needed for spinAroundDribbler https://github.com/kallaspriit/soccervision/blob/80840c921ad0935ed2e0718ed405613af3e51aa1/src/Robot.cpp#L385
	virtual void Drive(const cv::Point2d &speed, double angularSpeed = 0) = 0; /* x,y speed components */
	virtual bool BallInTribbler() = 0;
	virtual void Kick(int force = 800) = 0;
	virtual void ToggleTribbler(int speed) = 0;
	virtual std::string GetDebugInfo() = 0;

};

class IControlModule {
//	virtual bool Init(ICommunicationModule * pComModule, FieldState *pFieldState) = 0;
};
/*
class IAutoPilot
{
public:
	virtual void UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool ballInTribbler, bool sightObstructed, bool somethingOnWay, int borderDistance) = 0;
	virtual std::string GetDebugInfo() = 0;
	//virtual void Enable(bool enable) = 0;
};
*/
class IButtonClickListener
{
	virtual void OnStartButtonClicked() = 0;
	virtual void OnGateButtonClicked(int selectedGate) = 0;
};

extern std::map<OBJECT, std::string> OBJECT_LABELS;
typedef std::map<OBJECT, HSVColorRange> HSVColorRangeMap;
typedef std::map<OBJECT, cv::Mat> ThresholdedImages;

class ImageThresholder
{
protected:
	ThresholdedImages &thresholdedImages;
	HSVColorRangeMap &objectMap;
public:
	ImageThresholder(ThresholdedImages &images, HSVColorRangeMap &objectMap) : thresholdedImages(images), objectMap(objectMap){};
	virtual void Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList) = 0;
	virtual ~ImageThresholder(){};
};


#define sign(x) ((x > 0) - (x < 0))
