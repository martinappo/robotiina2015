#include "ObjectFinder.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <math.h>
void drawLine(cv::Mat & img, cv::Mat & img2, cv::Vec4f line, int thickness, CvScalar color, bool nightVision = false);

ObjectFinder::ObjectFinder()
{
	using boost::property_tree::ptree;
	try {

		ptree pt;
		read_ini("conf/camera.ini", pt);
		Vfov = pt.get<float>("Vfov");
		CamHeight = pt.get<float>("Height");
		CamAngleDev = pt.get<float>("AngleDev");
	}
	catch (...){
		ptree pt;
		pt.put("Height", CamHeight);
		pt.put("AngleDev", CamAngleDev);
		pt.put("Vfov", Vfov);
		write_ini("conf/camera.ini", pt);
	};
}

void drawLine(cv::Mat & img, cv::Mat & img2, cv::Vec4f line, int thickness, CvScalar color, bool nightVision/* = false*/)
{
	double theMult = std::max(img.cols, img.rows);
	// calculate start point
	cv::Point startPoint;
	startPoint.x = (int)(line[2] - theMult*line[0]);// x0
	startPoint.y = (int)(line[3] - theMult*line[1]);// y0
	// calculate end point
	cv::Point endPoint;
	endPoint.x = (int)(line[2] + theMult*line[0]);//x[1]
	endPoint.y = (int)(line[3] + theMult*line[1]);//y[1]

	// draw overlay of bottom lines on image
	cv::clipLine(cv::Size(img.cols, img.rows), startPoint, endPoint);
	std::vector <cv::Point2i> points;
	if (startPoint.x == 0) {
		points.push_back(cv::Point2i(0, 0));
	}
	if (endPoint.x == img.cols - 1) {
		points.push_back(cv::Point2i(img.cols - 1, 0));
	}
	if (points.size() == 0) {
		if (startPoint.y == 0 && endPoint.y == img.rows - 1) {
			points.push_back(cv::Point2i(img.cols - 1, 0));
		}
		else if (startPoint.y == img.rows - 1 && endPoint.y == 0) {
			points.push_back(cv::Point2i(0, 0));
		}
	}
	points.push_back(endPoint);
	points.push_back(startPoint);

	if (points.size() > 2) {
		if (nightVision) {
			cv::fillConvexPoly(img, points, cv::Scalar(0, 0, 0));
		}
		cv::fillConvexPoly(img2, points, cv::Scalar(0, 0, 0));
	}
	else {
		std::cout << "unable to fill line: " << startPoint << ", " << endPoint << std::endl;
	}
	return;
	

}
