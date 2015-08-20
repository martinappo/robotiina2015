#include "types.h"
#include "ThreadedClass.h"


class ImageThresholder : public ThreadedClass
{
protected:
	ThresholdedImages &thresholdedImages;
	HSVColorRangeMap &objectMap;
	cv::Mat frame;
public:
	void Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList);

	ImageThresholder(ThresholdedImages &images, HSVColorRangeMap &objectMap);
	~ImageThresholder();

	void Run() {};
	void Run2(OBJECT object);
private:
    cv::Mat elemDilate;
    cv::Mat elemErode;
    cv::Mat elemErode2;
	std::atomic_int m_iWorkersInProgress;

};

