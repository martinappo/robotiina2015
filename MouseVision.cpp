#include "MouseVision.h"
#include <chrono>
#include <thread>
#include "ObjectFinder.h"

MouseVision::MouseVision(ICamera * pCamera, IDisplay *pDisplay, FieldState *pFieldState) :ThreadedClass("MouseVision")
{
	m_pCamera = pCamera;
	m_pDisplay = pDisplay;
	m_pState = pFieldState;

	frame_size = m_pCamera->GetFrameSize();
	pDisplay->AddEventListener(this);

}


MouseVision::~MouseVision()
{
	m_pDisplay->RemoveEventListener(this);
	WaitForStop();

}

void MouseVision::Run(){
	RobotPosition &robotPos = m_pState->self;
	while (!stop_thread){
		cv::Scalar colorCircle(133, 33, 255);
		frameBGR = m_pCamera->Capture();
		cv::Scalar color(0, 0, 0);
		cv::circle(frameBGR, cv::Point(x, y), 8, colorCircle, -1);
		//cv::circle(frameBGR, cv::Point(frameBGR.cols - x, frameBGR.rows - y), 8, color, -1);

		frameBGR.copyTo(display);
		m_pState->blueGate.updateRawCoordinates(cv::Point2d(x, y), cv::Point2d(frameBGR.size()/2));
		//m_pState->yellowGate.updateCoordinates(cv::Point(frameBGR.cols - x, frameBGR.rows - y));
		m_pState->self.updateFieldCoords(cv::Point2d(0,0), 0);
		//ballFinder.populateBalls(thresholdedImages, frameHSV, frameBGR, BALL, m_pState);
		m_pDisplay->ShowImage(display);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}