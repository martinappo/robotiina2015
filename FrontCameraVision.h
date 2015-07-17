#pragma once
#include "types.h"
#include "ThreadedClass.h"

class FrontCameraVision :
	public IVisionModule, public ThreadedClass
{
protected:
	ICamera *m_pCamera;
	IDisplay *m_pDisplay;
	IFieldStateListener *m_pStateListener;
	FieldState state;

	cv::Mat frameBGR, frameHSV;
	HSVColorRangeMap objectThresholds;

	bool gaussianBlurEnabled = false;
	bool sonarsEnabled = false;
	bool greenAreaDetectionEnabled = false;
	bool gateObstructionDetectionEnabled = false;
	bool borderDetectionEnabled = false;
	bool nightVisionEnabled = false;

public:
	FrontCameraVision();
	bool Init(ICamera * pCamera, IDisplay *pDisplay, IFieldStateListener * pFieldStateListener);
	virtual ~FrontCameraVision();
	void Run();
	const cv::Mat & GetFrame() { return m_pCamera->Capture();  }

};

