#include "types.h"
#include "ThreadedClass.h"


class ImageThresholderOld : public ThreadedClass, cv::ParallelLoopBody
{
protected:
	ThresholdedImages &thresholdedImages;
	HSVColorRangeMap &objectMap;
	cv::Mat frame;
public:
	void Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList);

	ImageThresholderOld(ThresholdedImages &images, HSVColorRangeMap &objectMap);
	~ImageThresholderOld();

	void Run() {};
	void Run2(OBJECT object);
private:
    cv::Mat elemDilate;
    cv::Mat elemErode;
    cv::Mat elemErode2;
	std::atomic_int m_iWorkersInProgress;

};

