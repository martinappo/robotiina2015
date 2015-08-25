#pragma once
#include <opencv2/opencv.hpp>
#include <math.h> 
#include <functional>
#include <atomic>

#ifdef WIN32
	#define _WIN32_WINNT 0x0600 // vista for socket.cancel()
	#ifndef _WIN32_WINNT_WS08
		#define _WIN32_WINNT_WS08 // GetTickCount64 is missing in mingw, so emulate Windows Server 2008
	#endif
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

const int ID_COILGUN = 4;

enum OBJECT
{
    BALL = 0, BLUE_GATE, YELLOW_GATE, FIELD, INNER_BORDER, OUTER_BORDER, NUMBER_OF_OBJECTS, SIGHT_MASK
};


const int NUMBER_OF_BALLS = 11;

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
	STATE_END_OF_GAME /* leave this last*/
};

class IObjectPosition {
public:
	virtual void updateCoordinates(int x, int y, cv::Point robotFieldCoords, int robotAngle) = 0; // Takes raw coordinates of object from frame
	virtual void updatePolarCoords(int x, int y) = 0;
protected:
	virtual void updatePolarCoords() = 0; //Relative to robot
	virtual void updateFieldCoords(cv::Point robotFieldCoords) = 0; //Relative to field
};

class IUIEventListener {
public:
	// xy coordinates are from 0...1.0...
	virtual bool OnMouseEvent(int event, float x, float y, int flags) { return false; };
	virtual void OnKeyPress(char key) {};
};

class IDisplay {
public:
	virtual void ShowImage(const cv::Mat &image, bool main = true) = 0;
	virtual void AddEventListener(IUIEventListener *pEventListener) = 0;
	virtual void RemoveEventListener(IUIEventListener *pEventListener) = 0;
};

class ICamera
{
public:
	virtual cv::Mat & Capture() = 0;
	virtual cv::Size GetFrameSize() = 0;
	virtual double GetFPS() = 0;
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

class IWheelController {
public:
	virtual void Drive(double fowardSpeed, double direction, double angularSpeed) = 0;
	virtual const Speed & GetActualSpeed() = 0;
	virtual const Speed & GetTargetSpeed() = 0;

};
class ICoilGun {
public:
	virtual bool BallInTribbler() = 0;
	virtual void Kick() = 0;
	virtual void ToggleTribbler(bool start) = 0;

};

class ICommunicationModule : public IWheelController, public ICoilGun {
//	virtual bool Init(IWheelController * pWheels, ICoilGun *pCoilGun) = 0;

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


#define sign(x) ((x > 0) - (x < 0))
