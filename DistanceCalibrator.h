#pragma once
#include "ThreadedClass.h"
//#include "ConfigurableModule.h"
#include "ColorCalibrator.h"
#include <boost/property_tree/ptree.hpp>
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
	const static int VIEWING_DISTANCE = 210;
	const static int DISTANCE_CALIBRATOR_STEP = 10;

protected:
	cv::Mat bestLabels, clustered, centers;
	void mouseClicked(int x, int y, int flags);
	ICamera *m_pCamera;
	IDisplay *m_pDisplay;

private:
	cv::Point frame_size;
	int counter;
	boost::property_tree::ptree pt;
};