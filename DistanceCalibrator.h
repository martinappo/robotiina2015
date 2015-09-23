#pragma once
#include "ThreadedClass.h"
//#include "ConfigurableModule.h"
#include "ColorCalibrator.h"
#include <boost/property_tree/ptree.hpp>
#include <atomic>

class DistanceCalibrator :  public IUIEventListener{

public:
	DistanceCalibrator(ICamera * pCamera, IDisplay *pDisplay);

	~DistanceCalibrator();
	virtual bool OnMouseEvent(int event, float x, float y, int flags, bool bMainArea);
	static double calculateDistance(double centreX, double centreY, double x, double y);
	void start();
	void removeListener();
	const static int VIEWING_DISTANCE = 210;
	const static int DISTANCE_CALIBRATOR_STEP = 10;
	const static int CONF_SIZE = VIEWING_DISTANCE / DISTANCE_CALIBRATOR_STEP;
	std::string counterValue;
	void Enable(bool enable);
	std::string message ="";

protected:
	bool enabled = false;
	cv::Mat bestLabels, clustered, centers;
	void mouseClicked(int x, int y, int flags);
	void mouseClicked2(int x, int y, int flags);
	ICamera *m_pCamera;
	IDisplay *m_pDisplay;
	std::vector<std::tuple<cv::Point, cv::Point, std::string>> points;
	std::vector<std::tuple<cv::Point, cv::Point, std::string>>::iterator itPoints;

	void calculateDistances();
private:
	cv::Point frame_size;
	boost::property_tree::ptree pt;
	int counter;
};