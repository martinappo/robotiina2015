#pragma once
#include "ThreadedClass.h"
#include "types.h"
#include <boost/thread/mutex.hpp>

class SoccerField :
	public ThreadedClass, public FieldState
{
public:
	SoccerField(IDisplay *pDisplay);
	virtual ~SoccerField();
	void Run();
	virtual void SetTargetGate(OBJECT gate) {
		m_targetGate = gate;
	};
	virtual ObjectPosition GetTargetGate(){
		if (m_targetGate == BLUE_GATE) return blueGate;
		else if (m_targetGate == YELLOW_GATE) return yellowGate;
		else { assert(false); return ObjectPosition(); }
	};

private:
	std::atomic_int m_targetGate;
	IDisplay *m_pDisplay;
	const cv::Mat green = cv::Mat(480, 640, CV_8UC3, cv::Scalar(21, 188, 80));

	cv::Mat field = cv::Mat(480, 640, CV_8UC3, cv::Scalar::all(245)); // blink display
};

