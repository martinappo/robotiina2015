#include "MouseFinder.h"


MouseFinder::MouseFinder()
{
	cv::namedWindow("MouseFinder");
	cv::setMouseCallback("MouseFinder", [](int event, int x, int y, int flags, void* self) {
		((MouseFinder*)self)->mouseLocation = cv::Point2i(x,y);

	}, this);
}

cv::Point2i MouseFinder::LocateBallOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target)
{
	cv::Point2i point = filter->doFiltering(cv::Point2i(mouseLocation.x, mouseLocation.y));
	cv::Scalar colorCircle(133, 33, 55);
	cv::circle(frameBGR, point, 10, colorCircle, 3);
	cv::imshow("MouseFinder", frameBGR);
	return point;

}
MouseFinder::~MouseFinder()
{
}
