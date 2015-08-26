#include "ImageThresholder.h"
#include <chrono>
#include <thread>

#define EDSIZE 24
#define ERODESIZE 10

//#define IMAGETHRESHOLDER_PARALLEL_FOR
#define IMAGETHRESHOLDER_PARALLEL_THREADS

#ifdef IMAGETHRESHOLDER_PARALLEL_FOR
#include <ppl.h>
#endif 

ImageThresholder::ImageThresholder(ThresholdedImages &images, HSVColorRangeMap &objectMap) : ThreadedClass("ImageThresholder"), thresholdedImages(images), objectMap(objectMap)
{
	stop_thread = false;
	running = false;
	m_iWorkersInProgress = 0;

#if defined(IMAGETHRESHOLDER_PARALLEL_THREADS)
	for (auto objectRange : objectMap) {
		auto object = objectRange.first;
		//threads.create_thread(boost::bind(&ImageThresholder::Run2, this, objectRange.first));
		threads.add_thread(new boost::thread(&ImageThresholder::Run2, this, objectRange.first));

	}
#endif

	elemDilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(EDSIZE, EDSIZE)); //millega hiljem erode ja dilatet teha
	elemErode = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(EDSIZE + 6, EDSIZE + 6));
	elemErode2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ERODESIZE, ERODESIZE));

};

ImageThresholder::~ImageThresholder(){
	WaitForStop();
};


void ImageThresholder::Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList) {
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


void  ImageThresholder::Run2(OBJECT object){
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


