#pragma once
#include "colorcalibrator.h"

class AutoCalibrator : public ColorCalibrator {
public:
	AutoCalibrator(cv::Point frame_size);
	bool LoadFrame(cv::Mat &image);
	void reset() { 
		mask = cv::Mat(frame_size.y, frame_size.x, CV_8U, cv::Scalar::all(0));
		cv::rectangle(mask, cv::Point(0, 0), cv::Point(frame_size.x / 2, frame_size.y / 2), cv::Scalar::all(255), -1);
		frames = 0;
	};
	HSVColorRange GetObjectThresholds(int index, const std::string &name);

	~AutoCalibrator();
	int frames = 0;
	cv::Mat mask;
protected:
	cv::Mat bestLabels, clustered, centers;
	void DetectThresholds(int number_of_objects);
	void mouseClicked(int x, int y, int flags);
private:
    bool done;
	std::string name;
	int max_image_count = 4;
	cv::Point frame_size;



};