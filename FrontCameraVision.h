#pragma once
#include "types.h"
#include "ThreadedClass.h"
#include "ConfigurableModule.h"
#include "FieldState.h"
class StateMachine;
class VideoRecorder;
class FrontCameraVision :
	public ConfigurableModule, public IVisionModule, public ThreadedClass
{
protected:
	ICamera *m_pCamera;
	IDisplay *m_pDisplay;
	FieldState *m_pState;

	cv::Mat frameHSV;
	HSVColorRangeMap objectThresholds;

	bool gaussianBlurEnabled = false;
	bool sonarsEnabled = false;
	bool greenAreaDetectionEnabled = false;
	bool gateObstructionDetectionEnabled = false;
	bool borderDetectionEnabled = true;
	bool borderCollisonEnabled = false;
	bool fieldCollisonEnabled = false;
	bool nightVisionEnabled = false;
	bool detectOtherRobots = false;
	bool detectObjectsNearBall = false;
	bool hideUseless = false;
	bool useKalmanFilter = false;

	VideoRecorder *videoRecorder  = NULL;
	double time = 0;
	StateMachine *m_pSM = NULL;
public:
	FrontCameraVision(ICamera * pCamera, IDisplay *pDisplay, FieldState *pFieldState, StateMachine *pSM);
	virtual ~FrontCameraVision();
	void Run();
	const cv::Mat & GetFrame() { return m_pCamera->Capture();  }
	bool captureFrames();
	void captureFrames(bool start);
};

