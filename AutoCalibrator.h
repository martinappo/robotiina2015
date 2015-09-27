#pragma once
#include "ThreadedClass.h"
//#include "ConfigurableModule.h"
#include "ColorCalibrator.h"
#include <atomic>

class AutoCalibrator : public ColorCalibrator,
	/*public ConfigurableModule,*/ public IVisionModule, public ThreadedClass, public IUIEventListener{
	enum {
		LIVE_FEED = 0,
		GRAB_FRAME,
		CALIBRATION,
		THRESHOLDING,
		GET_THRESHOLD,
		CROPPING
	};

public:
	AutoCalibrator(ICamera * pCamera, IDisplay *pDisplay);
	void LoadFrame();
	void reset() { 
		boost::mutex::scoped_lock lock(mutex); //allow one command at a time
		resize(white, image, frame_size);
		//display = cv::Mat(frame_size.y, frame_size.x, CV_8U, cv::Scalar::all(255));
		white.copyTo(display);
		frames = 0;
		screenshot_mode = LIVE_FEED;

		cv::Point thresholdCorner1 = cv::Point(0, 0);
		cv::Point thresholdCorner2 = cv::Point(0, 0);
		bool drawRect = false;
	};
	HSVColorRange GetObjectThresholds(int index, const std::string &name);

	~AutoCalibrator();
	int frames = 0;
	void Run();
	const cv::Mat & GetFrame() { return m_pCamera->Capture(); }
	virtual bool OnMouseEvent(int event, float x, float y, int flags, bool bMainArea);

protected:
	cv::Mat bestLabels, clustered, centers;
	void DetectThresholds(int number_of_objects);
	void mouseClicked(int x, int y, int flags);

	ICamera *m_pCamera;
	IDisplay *m_pDisplay;
	bool m_bEnableCapture;
	cv::Mat frameBGR, frameHSV, display;
	const cv::Mat white = cv::Mat(480, 640, CV_8UC3, cv::Scalar::all(245)); // blink display

private:
    bool done;
	std::string object_name;
	cv::Point frame_size;
	boost::mutex mutex;
	std::atomic_int screenshot_mode;
	cv::Point thresholdCorner1 = cv::Point(0, 0);
	cv::Point thresholdCorner2 = cv::Point(0, 0);
	bool drawRect = false;

};