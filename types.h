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

struct ObjectPosition /* polar coordinates */
{
	double distance;
	double horizontalDev; // perhaps not needed
	double horizontalAngle;
};

enum OBJECT_ID {
	ID_WHEEL_LEFT = 1,
	ID_WHEEL_RIGHT = 2,
	ID_WHEEL_BACK = 3,
	ID_COILGUN = 4,
	ID_OBJECT_COUNT
};
const int ID_AUDRINO = 5;
enum OBJECT
{
    BALL = 0, GATE1, GATE2, FIELD, INNER_BORDER, OUTER_BORDER, NUMBER_OF_OBJECTS, SIGHT_MASK
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

struct FieldState {
	ObjectPosition self; // our robot distance from center and rotation
	ObjectPosition opponent; // distance from center and rotation
	ObjectPosition balls[NUMBER_OF_BALLS];
	ObjectPosition selfGate;
	ObjectPosition opponentGate;
};

class IFieldStateListener {
	virtual void OnFieldStateChanged(const FieldState &state) = 0;
};

class IDisplay {
public:
	virtual void ShowImage(const cv::Mat image) = 0;
};

class ICamera
{
public:
	virtual const cv::Mat & Capture() = 0;
};

class IVisionModule {
public:
	virtual bool Init(ICamera * pCamera, IDisplay *pDisplay, IFieldStateListener * pFieldStateListener) = 0;
	virtual const cv::Mat & GetFrame() = 0;
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
