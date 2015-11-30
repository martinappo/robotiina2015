#include "VideoRecorder.h"
#include <boost/filesystem.hpp>

#ifdef WIN32
//#define SAVE_AS_AVI
#endif

VideoRecorder::VideoRecorder(const std::string &outputDir, int fps, const cv::Size &frameSize) : outputDir(outputDir), fps(fps), frameSize(frameSize)
{
	isRecording = false;
}


VideoRecorder::~VideoRecorder()
{
	Stop();
}

void VideoRecorder::Start()
{
	Stop();
	isRecording = true;
	outputVideo = new cv::VideoWriter();
	subtitles = new std::ofstream();
	boost::posix_time::ptime captureStart = boost::posix_time::microsec_clock::local_time();
	fileName = outputDir + boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time());
	std::replace(fileName.begin(), fileName.end(), ':', '.');

#ifdef SAVE_AS_AVI
#ifdef WIN32
	int ex = -1;
#else
	int ex = CV_FOURCC('x','v','i','d'); //CV_FOURCC('F', 'M', 'P', '4');
#endif
	std::cout << "Save video: " << fileName << ".avi" << ", ex: " << ex << ", fps: " << fps << ", size: " << frameSize << std::endl;
	outputVideo->open(fileName + ".avi", ex, fps, frameSize, true);
	if (!outputVideo->isOpened())
	{
		std::cout << "Could not open the output video for write: " << fileName << ".avi" << ", size: " << frameSize << std::endl;
	}
#else 
	boost::filesystem::create_directories(fileName);

	std::cout << "Save video: " << fileName << "" << std::endl;
#endif
	subtitles->open(fileName + ".sub");
	frameCounter = 1;


}
void padTo(std::string &str, const size_t num, const char paddingChar = '0')
{
    if(num > str.size())
    	str.insert(0, num - str.size(), paddingChar);
}

void VideoRecorder::Stop()
{
	isRecording = false;
	if (outputVideo != NULL) {
		delete outputVideo;
		outputVideo = NULL;
	}
	if (subtitles != NULL) {
		subtitles->close();
		delete subtitles;
		subtitles = NULL;
	}

}
void VideoRecorder::RecordFrame(const cv::Mat &frame, const std::string subtitle)
{
#ifdef SAVE_AS_AVI
	*outputVideo << frame;
#else
	
	std::string f = std::to_string(frameCounter);
	padTo(f,16);
	std::string frameName = fileName + "/" + f + ".jpg";
	std::replace(frameName.begin(), frameName.end(), ':', '.');
	cv::imwrite(frameName, frame);
#endif
	if (subtitles != NULL)
		*subtitles << "{" << frameCounter << "}{" << (frameCounter) << "}" << " frame " << frameCounter << ": " << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time()) << "|" << subtitle << "\r\n";
	frameCounter++;
}
