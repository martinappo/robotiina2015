#include "MouseVision.h"
#include <chrono>
#include <thread>
#include "ObjectFinder.h"

MouseVision::MouseVision(ICamera * pCamera, IDisplay *pDisplay, FieldState *pFieldState)
{
	m_pCamera = pCamera;
	m_pDisplay = pDisplay;
	m_pState = pFieldState;

	frame_size = m_pCamera->GetFrameSize();
	pDisplay->AddEventListener(this);

}


MouseVision::~MouseVision()
{
	WaitForStop();
	m_pDisplay->RemoveEventListener(this);

}

void MouseVision::Run(){
	ObjectFinder finder;
	BallPosition targetPos;
	while (!stop_thread){
		cv::Scalar colorCircle(133, 33, 255);
		frameBGR = m_pCamera->Capture();
		frameBGR.copyTo(display);
		finder.LocateCursor(display, cv::Point(x, y), BALL, targetPos);
		m_pState->balls[0] = targetPos;
		m_pDisplay->ShowImage(display);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
}