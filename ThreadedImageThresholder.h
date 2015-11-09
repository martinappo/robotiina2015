#pragma once
#include "types.h"
#include <chrono>
#include <thread>
#include "ThreadedClass.h"
class ThreadedImageThresholder :
	public ThreadedClass, public ImageThresholder
{
public:
	ThreadedImageThresholder(ThresholdedImages &images, HSVColorRangeMap &objectMap) : ThreadedClass("ImageThresholder"), ImageThresholder(images, objectMap){
		m_iWorkersInProgress = 0;
		stop_thread = false;
		running = false;
		m_iWorkersInProgress = 0;
		for (auto objectRange : objectMap) {
			auto object = objectRange.first;
			threads.add_thread(new boost::thread(&ThreadedImageThresholder::Run2, this, objectRange.first));

		}

	}

	~ThreadedImageThresholder(){
		WaitForStop();
	};

	void Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList) {
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
	}
	void Run() {};
	void  Run2(OBJECT object){
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

protected:
	cv::Mat frame;
	std::atomic_int m_iWorkersInProgress;

};

