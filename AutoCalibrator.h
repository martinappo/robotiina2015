#pragma once
#include "ThreadedClass.h"
//#include "ConfigurableModule.h"
#include "ColorCalibrator.h"

class AutoCalibrator : public ColorCalibrator,
	/*public ConfigurableModule,*/ public IVisionModule, public ThreadedClass{
public:
	AutoCalibrator();
	bool LoadFrame();
	void reset() { 
		mask = cv::Mat(frame_size.y, frame_size.x, CV_8U, cv::Scalar::all(0));
		cv::rectangle(mask, cv::Point(0, 0), cv::Point(frame_size.x / 2, frame_size.y / 2), cv::Scalar::all(255), -1);
		frames = 0;
	};
	HSVColorRange GetObjectThresholds(int index, const std::string &name);
	bool Init(ICamera * pCamera, IDisplay *pDisplay, IFieldStateListener * pFieldStateListener);

	~AutoCalibrator();
	int frames = 0;
	cv::Mat mask;
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

private:
    bool done;
	std::string name;
	int max_image_count = 4;
	cv::Point frame_size;



};