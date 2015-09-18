#include "ImageThresholder.h"
#include <chrono>
#include <thread>

#define EDSIZE 24
#define ERODESIZE 10

//#define IMAGETHRESHOLDER_PARALLEL_FOR
//#define IMAGETHRESHOLDER_PARALLEL_THREADS
#define IMAGETHRESHOLDER_PARALLEL_INRANGE

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
#elif defined(IMAGETHRESHOLDER_PARALLEL_INRANGE)
	for (auto &object : objectList) {
		thresholdedImages[object] = cv::Mat(frameHSV.rows, frameHSV.cols, CV_8U, cv::Scalar::all(0));
	}
	std::map<OBJECT, uchar*> pMap;
	for (int row = 0; row < frameHSV.rows; ++row) {
		uchar * p_src = frameHSV.ptr(row);
		for (auto &object : objectList) {
			pMap[object] = thresholdedImages[object].ptr(row);
		}
		for (int col = 0; col < frameHSV.cols; ++col) {
			int srcH = *p_src++;
			int srcS = *p_src++;
			int srcV = *p_src++;
			for (auto &object : objectList) {
				auto r = objectMap[object];
				int lhue = r.hue.low;
				int hhue = r.hue.high;
				int lsat = r.sat.low;
				int hsat = r.sat.high;
				int lval = r.val.low;
				int hval = r.val.high;
				if (srcH >= lhue && srcH <= hhue &&
					srcS >= lsat && srcS <= hsat &&
					srcV >= lval && srcV <= hval) {
					*(pMap[object]) = 255;
				}
				(*pMap[object])++;
			}
		}
		/*
		for (int i = 0; i < frameHSV.rows; i++) {
			for (int j = 0; j < frameHSV.cols; j++) {
				cv::Vec3b p = frameHSV.at<cv::Vec3b>(i, j);
				if (p[0] >= lhue && p[0] <= hhue &&
					p[1] >= lsat && p[1] <= hsat &&
					p[2] >= lval && p[2] <= hval) {
					thresholdedImages[object].at<unsigned char>(i, j) = 255;
				}
			}
		}
		*/
}
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


