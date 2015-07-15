#include "types.h"
#define EDSIZE 24
#define ERODESIZE 10

class ImageThresholder : public ThreadedClass
{
protected:
	ThresholdedImages &thresholdedImages;
	HSVColorRangeMap &objectMap;
public:
	void Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList) {
    
		for (auto &object : objectList) {
			threads.create_thread([&frameHSV, object, this]{
				auto r = objectMap[object];
				do {
					inRange(frameHSV, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), thresholdedImages[object]);
				} while (thresholdedImages[object].size().height == 0);
  	         if (object == GATE1 || object == GATE2) {
	            cv::erode(thresholdedImages[object],thresholdedImages[object],elemErode2);
	            }
 	           cv::dilate(thresholdedImages[object],thresholdedImages[object],elemErode2);
 
			});
		}
	}
	ImageThresholder(ThresholdedImages &images, HSVColorRangeMap &objectMap) : ThreadedClass("ImageThresholder"), thresholdedImages(images), objectMap(objectMap){
	
    elemDilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(EDSIZE,EDSIZE)); //millega hiljem erode ja dilatet teha
    elemErode = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(EDSIZE+6,EDSIZE+6));
    elemErode2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ERODESIZE,ERODESIZE));
	
	};
	~ImageThresholder(){};

	void Run(){};
private:
    cv::Mat elemDilate;
    cv::Mat elemErode;
    cv::Mat elemErode2;

};

