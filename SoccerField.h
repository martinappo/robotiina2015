#pragma once
#include "ThreadedClass.h"
#include "types.h"
#include <boost/thread/mutex.hpp>
#include "FieldState.h"

class SoccerField :
	public ThreadedClass, public FieldState
{
public:
	SoccerField(IDisplay *pDisplay, cv::Size frameSize);
	virtual ~SoccerField();
	void Run();
	virtual void SetTargetGate(OBJECT gate) {
		m_targetGate = gate;
	};
	virtual ObjectPosition GetTargetGate() const;
	void initBalls(cv::Size frameSize);
private:
	std::atomic_int m_targetGate;
	IDisplay *m_pDisplay;
	const cv::Mat green = cv::Mat(480, 640, CV_8UC3, cv::Scalar(21, 188, 80));
	cv::Mat field = cv::Mat(480, 640, CV_8UC3, cv::Scalar::all(245)); // blink display
};

