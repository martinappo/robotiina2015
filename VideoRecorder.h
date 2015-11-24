#pragma once
#include "types.h"
#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <iostream>
#include <fstream>

class VideoRecorder
{
private:
	const std::string outputDir;
	std::string fileName;
	int fps;
	const cv::Size frameSize;
	cv::VideoWriter *outputVideo = NULL;
	std::ofstream *subtitles = NULL;
	boost::posix_time::ptime captureStart = boost::posix_time::microsec_clock::local_time();
	int frameCounter = 0;

public:
	VideoRecorder(const std::string &outputDir, int fps, const cv::Size &frameSize);
	void Start();
	void Stop();
	void RecordFrame(const cv::Mat &frame, const std::string subtitle);
	~VideoRecorder();
	bool isRecording;
};
