#pragma once
#include "ThreadedClass.h"
//#include "ConfigurableModule.h"
#include "ColorCalibrator.h"
#include <atomic>

class DistanceCalibrator :  public IUIEventListener{

public:
	DistanceCalibrator(ICamera * pCamera, IDisplay *pDisplay);

	bool distanceCalibrationRunning = false;
	~DistanceCalibrator();
	virtual bool OnMouseEvent(int event, float x, float y, int flags);	
	static double calculateDistance(double centreX, double centreY, double x, double y);
	void start();
	void removeListener();

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