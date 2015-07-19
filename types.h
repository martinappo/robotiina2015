#pragma once
#include <opencv2/opencv.hpp>
#include <math.h> 
#ifdef WIN32
	#define _WIN32_WINNT 0x0600 // vista for socket.cancel()
	#ifndef _WIN32_WINNT_WS08
		#define _WIN32_WINNT_WS08 // GetTickCount64 is missing in mingw, so emulate Windows Server 2008
	#endif
#endif
#define PI 3.14159265

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

struct ObjectPosition
{
	double distance;
	double horizontalDev;
	double horizontalAngle;
};

enum WHEEL_ID {
	ID_WHEEL_FRONT_1 = 1,
	ID_WHEEL_FRONT_2 = 2,
	ID_WHEEL_BACK_1 = 3,
	ID_WHEEL_BACK_2 = 4,
	WHEEL_COUNT
};

const int ID_COILGUN = 4;
const int ID_ARDUINO = 5;
enum OBJECT
{
    BALL = 0, GATE1, GATE2, FIELD, INNER_BORDER, OUTER_BORDER, NUMBER_OF_OBJECTS, SIGHT_MASK
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
	/* autopilot states
	STATE_LOCATE_BALL,
    STATE_LOCATE_GATE,
	STATE_CRASH,
	*/
	STATE_REMOTE_CONTROL,
	STATE_MANUAL_CONTROL,
	STATE_DANCE,
	STATE_TEST,
	STATE_TEST_COILGUN,
	STATE_END_OF_GAME /* leave this last*/
};

class ICamera
{
public:
    virtual const cv::Mat & Capture() = 0;
};

class IAutoPilot
{
public:
	virtual void UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool ballInTribbler, bool sightObstructed, bool somethingOnWay, int borderDistance) = 0;
	virtual std::string GetDebugInfo() = 0;
	virtual ~IAutoPilot(){}
};

class IButtonClickListener
{
	virtual void OnStartButtonClicked() = 0;
	virtual void OnGateButtonClicked(int selectedGate) = 0;
};

extern std::map<OBJECT, std::string> OBJECT_LABELS;
typedef std::map<OBJECT, HSVColorRange> HSVColorRangeMap;
typedef std::map<OBJECT, cv::Mat> ThresholdedImages;


#define sign(x) ((x > 0) - (x < 0))
