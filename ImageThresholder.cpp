#include "ImageThresholder.h"
#include <chrono>
#include <thread>
#include <algorithm>  

#define EDSIZE 24
#define ERODESIZE 10

//#define IMAGETHRESHOLDER_PARALLEL_FOR
#define IMAGETHRESHOLDER_PARALLEL_THREADS
//#define IMAGETHRESHOLDER_PARALLEL_INRANGE

#ifdef IMAGETHRESHOLDER_PARALLEL_FOR
#include <ppl.h>
#endif 

ImageThresholderOld::ImageThresholderOld(ThresholdedImages &images, HSVColorRangeMap &objectMap) : ThreadedClass("ImageThresholderOld"), thresholdedImages(images), objectMap(objectMap)
{
	stop_thread = false;
	running = false;
	m_iWorkersInProgress = 0;

#if defined(IMAGETHRESHOLDER_PARALLEL_THREADS)
	for (auto objectRange : objectMap) {
		auto object = objectRange.first;
		//threads.create_thread(boost::bind(&ImageThresholderOld::Run2, this, objectRange.first));
		threads.add_thread(new boost::thread(&ImageThresholderOld::Run2, this, objectRange.first));

	}
#endif

	elemDilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(EDSIZE, EDSIZE)); //millega hiljem erode ja dilatet teha
	elemErode = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(EDSIZE + 6, EDSIZE + 6));
	elemErode2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ERODESIZE, ERODESIZE));

};

ImageThresholderOld::~ImageThresholderOld(){
	WaitForStop();
};


void ImageThresholderOld::Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList) {
#if defined(IMAGETHRESHOLDER_PARALLEL_FOR)
		concurrency::parallel_for_each(begin(objectList), end(objectList), [&frameHSV, this](OBJECT object) {
			auto r = objectMap[object];
			inRange(frameHSV, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), thresholdedImages[object]);
		});
#elif defined(IMAGETHRESHOLDER_PARALLEL_THREADS)
	if (m_iWorkersInProgress > 0) {
		std::cout << "Still working" << std::endl;
	}
	frame = frameHSV;
	int mask = 0;
	for (auto &object : objectList) {
		//std::cout << "mask " << (1 << object) << " " <<( mask | (1 << object)) << std::endl;
		mask = mask | (1 << object);
	}
	m_iWorkersInProgress = mask;

	while (m_iWorkersInProgress > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1)); // limit fps to about 50fps
	}
	/*
		for (auto &object : objectList) {
			threads.create_thread([&frameHSV, object, this]{
				auto r = objectMap[object];
				do {
					inRange(frameHSV, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), thresholdedImages[object]);
				} while (thresholdedImages[object].size().height == 0);
			});
		}
	*/
#elif defined(IMAGETHRESHOLDER_PARALLEL_INRANGE)

	auto &rb = objectMap[BALL];
	auto &rbg = objectMap[BLUE_GATE];
	auto &ryg = objectMap[YELLOW_GATE];

	for (int i = 0; i < frameHSV.cols*frameHSV.rows * 3; i += 3) { 
		int h = frameHSV.data[i];
		int s = frameHSV.data[i + 1];
		int v = frameHSV.data[i + 2];



		bool ball = (rb.hue.low < h) && (rb.hue.high > h) && (rb.sat.low < s) && (rb.sat.high > s) && (rb.val.low < v) && (rb.val.high > v);
		bool blue = (rbg.hue.low < h) && (rbg.hue.high > h) && (rbg.sat.low < s) && (rbg.sat.high > s) && (rbg.val.low < v) && (rbg.val.high > v);
		bool yellow = (ryg.hue.low < h) && (ryg.hue.high > h) && (ryg.sat.low < s) && (ryg.sat.high > s) && (ryg.val.low < v) && (ryg.val.high > v);

		frameHSV.data[i] = ball ? 255 : 0;
		frameHSV.data[i + 1] = blue ? 255 : 0;
		frameHSV.data[i + 2] = yellow ? 255 : 0;


	}
	cv::extractChannel(frameHSV, thresholdedImages[BALL], 0);
	cv::extractChannel(frameHSV, thresholdedImages[BLUE_GATE], 1);
	cv::extractChannel(frameHSV, thresholdedImages[YELLOW_GATE], 2);


#else
		for (auto &object : objectList) {
			auto r = objectMap[object];
			inRange(frameHSV, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), thresholdedImages[object]);
		}
#endif
		/*
		if (object == BLUE_GATE || object == YELLOW_GATE) {
		cv::erode(thresholdedImages[object],thresholdedImages[object],elemErode2);
		}
		cv::dilate(thresholdedImages[object],thresholdedImages[object],elemErode2);

		*/
	}


void  ImageThresholderOld::Run2(OBJECT object){
	while (!stop_thread) {
		if (m_iWorkersInProgress & (1 << object)) {
			auto r = objectMap[object];
			inRange(frame, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), thresholdedImages[object]);
			m_iWorkersInProgress &= ~(1 << object);
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
};
