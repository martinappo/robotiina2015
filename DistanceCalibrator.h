#pragma once
#include "ThreadedClass.h"
//#include "ConfigurableModule.h"
#include "ColorCalibrator.h"
#include <atomic>

class DistanceCalibrator :  public IUIEventListener{

public:
	DistanceCalibrator(ICamera * pCamera, IDisplay *pDisplay);

	~DistanceCalibrator();
	virtual bool OnMouseEvent(int event, float x, float y, int flags);
	bool distanceCalibrationRunning = false;
	double calculateDistance(double centreX, double centreY, double x, double y);

protected:
	cv::Mat bestLabels, clustered, centers;
	void mouseClicked(int x, int y, int flags);
	ICamera *m_pCamera;
	IDisplay *m_pDisplay;

private:
	cv::Point frame_size;
	int counterValue = 5;
	int counter = counterValue;
};