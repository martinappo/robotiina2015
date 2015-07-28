#pragma once
#include "ThreadedClass.h"
//#include "ConfigurableModule.h"
#include "ColorCalibrator.h"
#include <atomic>

class AutoCalibrator : public ColorCalibrator,
	/*public ConfigurableModule,*/ public IVisionModule, public ThreadedClass{
public:
	AutoCalibrator();
	bool LoadFrame();
	void reset() { 
		image = cv::Mat(frame_size.y, frame_size.x, CV_8U, cv::Scalar::all(255));
		frames = 0;
		screenshot_mode = true;
	};
	HSVColorRange GetObjectThresholds(int index, const std::string &name);
	bool Init(ICamera * pCamera, IDisplay *pDisplay, IFieldStateListener * pFieldStateListener);

	~AutoCalibrator();
	int frames = 0;
	void Run();
	const cv::Mat & GetFrame() { return m_pCamera->Capture(); }

protected:
	cv::Mat bestLabels, clustered, centers;
	void DetectThresholds(int number_of_objects);
	void mouseClicked(int x, int y, int flags);

	ICamera *m_pCamera;
	IDisplay *m_pDisplay;
	bool m_bEnableCapture;
	cv::Mat frameBGR, frameHSV;
	const cv::Mat white = cv::Mat(10, 10, CV_8U, cv::Scalar::all(255)); // blink display

private:
    bool done;
	std::string name;
	int max_image_count = 4;
	cv::Point frame_size;
	boost::mutex mutex;
	std::atomic_bool screenshot_mode;



};