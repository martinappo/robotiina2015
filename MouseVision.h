#pragma once
#include "types.h"
#include "ThreadedClass.h"
#include "FieldState.h"

class MouseVision :
	public IVisionModule, public IUIEventListener, public ThreadedClass
{
public:
	MouseVision(ICamera * pCamera, IDisplay *pDisplay, FieldState *pFieldState);
	virtual ~MouseVision();
	virtual bool OnMouseEvent(int event, float x, float y, int flags, bool bMainArea){
		if (!running) return false;
		this->x = (int)(x * frame_size.x);
		this->y = (int)(y * frame_size.y);
		return false;
	}
protected:
	ICamera *m_pCamera;
	IDisplay *m_pDisplay;
	FieldState *m_pState;

	cv::Mat frameBGR, display;
	void Run();
	const cv::Mat & GetFrame() { return m_pCamera->Capture(); }
	std::atomic_int x, y;
	cv::Point frame_size;

};

