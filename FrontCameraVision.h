#pragma once
#include "types.h"
#include "ThreadedClass.h"
#include "ConfigurableModule.h"

class FrontCameraVision :
	public ConfigurableModule, public IVisionModule, public ThreadedClass
{
protected:
	ICamera *m_pCamera;
	IDisplay *m_pDisplay;
	FieldState *m_pState;

	cv::Mat frameBGR, frameHSV;
	HSVColorRangeMap objectThresholds;

	bool gaussianBlurEnabled = false;
	bool sonarsEnabled = false;
	bool greenAreaDetectionEnabled = false;
	bool gateObstructionDetectionEnabled = false;
	bool borderDetectionEnabled = false;
	bool nightVisionEnabled = false;

public:
	FrontCameraVision(ICamera * pCamera, IDisplay *pDisplay, FieldState *pFieldState);
	virtual ~FrontCameraVision();
	void Run();
	const cv::Mat & GetFrame() { return m_pCamera->Capture();  }

};

