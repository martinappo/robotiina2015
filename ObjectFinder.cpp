#include "objectfinder.h"
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
bool ObjectFinder::LocateCursor(cv::Mat &frameBGR, cv::Point2i cursor, OBJECT target, ObjectPosition &targetPos){

	cv::Scalar color(0, 0, 0);
	cv::circle(frameBGR, cursor, 8, color, -1);
	//std::cout << point << std::endl;
	targetPos = ConvertPixelToRealWorld(cursor, cv::Point2i(frameBGR.cols, frameBGR.rows));
	WriteInfoOnScreen(targetPos);
	return true;


}


bool ObjectFinder::Locate(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, ObjectPosition &targetPos) {
	cv::Point2i point(-1, -1);
	cv::Scalar color(0, 0, 0);
	cv::Scalar color2(255, 0, 255);
	bool resetFilter = false;
	point = LocateOnScreen(HSVRanges, frameHSV, frameBGR, target);
	/*
	if (target == BALL){
		point = LocateBallOnScreen(HSVRanges, frameHSV, frameBGR, target);
		color = cv::Scalar(0, 225, 225);
	}
	else{
		point = LocateGateOnScreen(HSVRanges, frameHSV, frameBGR, target);
		cv::circle(frameBGR, point, 8, color2, -1);
	}
	*/
	if (point.x < -1 && point.y < -1){//If ball is not valid then no predicting
		lastPosition = point;
		resetFilter = true;
		return false;
	}
	else if (point.x < 0 && point.y < 0){//If ball is suddenly lost then predict where it could be
		point = filter->getPrediction();
		lastPosition = point;
		if (point.x < 0 && point.y < 0){
			resetFilter = true;
			return false;
		}
	}
	else {//Ball is in frame
		if (resetFilter){
			resetFilter = false;
			filter->reset(point);
		}
		point = filter->doFiltering(point);
		lastPosition = point;
	}
	//cv::circle(frameBGR, point, 8, color, -1);
	//std::cout << point << std::endl;
	targetPos = ConvertPixelToRealWorld(point, cv::Point2i(frameHSV.cols, frameHSV.rows));
	WriteInfoOnScreen(targetPos);
	return true;
}


void drawLine(cv::Mat & img, cv::Mat & img2, cv::Vec4f line, int thickness, CvScalar color, bool nightVision/* = false*/)
{
	double theMult = std::max(img.cols, img.rows);
	// calculate start point
	cv::Point startPoint;
	startPoint.x = line[2] - theMult*line[0];// x0
	startPoint.y = line[3] - theMult*line[1];// y0
	// calculate end point
	cv::Point endPoint;
	endPoint.x = line[2] + theMult*line[0];//x[1]
	endPoint.y = line[3] + theMult*line[1];//y[1]

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
	cv::line(img, startPoint, endPoint, color, thickness, 8, 0);
	return;
	

}
int ObjectFinder::IsolateField(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, bool detectBothBorders/* = false*/, bool nightVision/* = false*/) {


	std::vector<cv::Vec4i> lines;
	std::vector<cv::Vec4i> lines2;

	cv::Vec4f newLine1;
	cv::Vec4f newLine2;
	float distance = INT_MAX;
	//Canny(HSVRanges[OUTER_BORDER], HSVRanges[OUTER_BORDER], 50, 200, 3);
	//cv::imshow("aaa", HSVRanges[OUTER_BORDER]);
	cv::HoughLinesP(HSVRanges[OUTER_BORDER], lines2, 1, CV_PI / 180, 80, 30, 10);
	double last_angle = 999;
	if (!detectBothBorders){
		for (auto l2 : lines2){
			std::vector<cv::Point2i> points2;
			points2.push_back(cv::Point(l2[0], l2[1]));
			points2.push_back(cv::Point(l2[2], l2[3]));
			cv::fitLine(points2, newLine2, CV_DIST_L2, 0, 0.1, 0.1);
			if (last_angle != 999 && abs(last_angle - atan2(newLine2[1], newLine2[0]) * 180 / PI) < 15) continue; // same border
			last_angle = atan2(newLine2[1], newLine2[0]) * 180 / PI;
			drawLine(frameBGR, HSVRanges[BALL], newLine2, 1, cv::Scalar(255 * (1 + 0.3), 0, 0), nightVision);
			distance = std::min(distance, newLine2[3] - (frameHSV.cols / 2)*newLine2[1]);
		}
		return distance;
	}
	cv::HoughLinesP(HSVRanges[INNER_BORDER], lines, 1, CV_PI / 180, 50, 50, 10);
//	cv::HoughLinesP(HSVRanges[INNER_BORDER] + HSVRanges[FIELD], lines, 1, CV_PI / 180, 50, 50, 10);


	//std::cout << rect_points[j] << ", " << rect_points[(j + 1) % 4] << ": " << atan2(newLine[1], newLine[0])*180/PI << std::endl;

	for (auto l1 : lines2){
		std::vector<cv::Point2i> points;
		points.push_back(cv::Point(l1[0], l1[1]));
		points.push_back(cv::Point(l1[2], l1[3]));
		cv::fitLine(points, newLine1, CV_DIST_L2, 0, 0.1, 0.1);
		for (auto l2 : lines){
			std::vector<cv::Point2i> points2;
			points2.push_back(cv::Point(l2[0], l2[1]));
			points2.push_back(cv::Point(l2[2], l2[3]));
			cv::fitLine(points2, newLine2, CV_DIST_L2, 0, 0.1, 0.1);
			if (abs(atan2(newLine1[1], newLine1[0]) - atan2(newLine2[1], newLine2[0])) < 0.1) { // almost parallel
				//cv::line(frameBGR, cv::Point(l1[0], l1[1]), cv::Point(l1[2], l1[3]), cv::Scalar(255, 0, 255), 3, CV_AA);
				drawLine(frameBGR, HSVRanges[BALL], newLine1, 1, cv::Scalar(255 * (1 + 0.3), 0, 0), nightVision);

			}
		}
	}

	return distance;


}

void ObjectFinder::IsolateFieldOld(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR) {

	cv::Mat innerThresholded = HSVRanges[INNER_BORDER];
	cv::Mat outerThresholded = HSVRanges[OUTER_BORDER];

	/*
	cv::Mat gate1Thresholded;
	inRange(frameHSV, cv::Scalar(gate1.hue.low, gate1.sat.low, gate1.val.low), cv::Scalar(gate1.hue.high, gate1.sat.high, gate1.val.high), gate1Thresholded); //Threshold the image
	cv::Mat gate2Thresholded;
	inRange(frameHSV, cv::Scalar(gate2.hue.low, gate2.sat.low, gate2.val.low), cv::Scalar(gate2.hue.high, gate2.sat.high, gate2.val.high), gate2Thresholded); //Threshold the image
	*/


	/* 
	cv::Point2d orgin(frameBGR.cols / 2, frameBGR.rows *0.9);
	cv::circle(frameBGR, orgin, 40, cv::Scalar(40, 20, 100), 10);
	for (float a = -PI; a < 0 ; a += PI/10) {
		bool was_border_start = false;
		for (float d = 0; d < frameBGR.cols ; d += 10) {
			float x = d * std::cos(a);
			float y = d * std::sin(a);
			if (abs(x) > orgin.x) break;
			if (y < -orgin.y) break;

			cv::Point2i point = orgin + cv::Point2d(x, y);
			bool border = abs(x) > orgin.x * 0.9 || y < -orgin.y*0.9;
			if (!border) {
				bool is_border_start = innerThresholded.ptr<uchar>(point.y)[point.x] == 255;
				bool is_border_end = outerThresholded.ptr<uchar>(point.y)[point.x] == 255;

				border = !is_border_start && is_border_end && was_border_start;
				was_border_start = is_border_start;
			}
			cv::circle(frameBGR, point, 4, border? cv::Scalar(d, 255, 100 * a)  : cv::Scalar(d, 20, 100 * a), 10);
			if (border) break;
		}
		//break;
	}
	*/

//	cv::Mat gateThresholded = gate1Thresholded + gate2Thresholded;

	//dir: left -> right, right -> left, top -> down
	for (int dir = 0; dir < 2; dir++) {
		std::vector<cv::Point2i> points;
		int end = dir ? frameHSV.cols - 1 : 0;
		//int last_y = -1;
		cv::Point2i last_canditate(-1, -1);
		int dx = 0;
		cv::Point2i last_selected(-1, -1);
		for (int y = 0; y < frameHSV.rows; y += 5) {
			int outer_start = -1;
			int outer_end = -1;
			int inner_start = -1;
			int inner_end = -1;
			bool break2 = false;
			for (int x = 0; x < frameHSV.cols; x += 3) { // TODO: start near the last found piont
				//cv::circle(frameBGR, cv::Point2i(dir ? end - x : x, y), 1, dir ? cv::Scalar(255, 0, 0) : cv::Scalar(120, 0, 0), 10);
				if (inner_end > 0 && inner_end - inner_start > 1) {
					// we have detected new point on the border
					if (last_canditate.x != -1) {
						// check the distance form last candidate
						if (abs(y - last_canditate.y) < 40 && abs(x - last_canditate.x) < 40) { // close enough
							cv::Point2i new_point = cv::Point2i(dir ? end - last_selected.x : last_selected.x, last_selected.y);
							cv::circle(frameBGR, new_point, 4, dir ? cv::Scalar(255, 255, 100) : cv::Scalar(0, 20, 100), 10);
							points.push_back(new_point);
							dx = last_selected.x - outer_start;
							if (points.size() > 7) {
								break2 = true;
								break;
							}
						}
						else {
							// to big gap, either restart or stop
							if (points.size() < 3) {
								points.clear();
								cv::Point2i last_canditate(-1, -1);
								cv::Point2i last_selected(-1, -1);
								//break;
							}
							else {
								break2 = true;
								break;
							}
						}

					}
					{
						last_canditate = cv::Point2i(x, y);
						last_selected = cv::Point2i(inner_start, y);
						//cv::Point2i new_point = cv::Point2i(dir ? end - last_selected.x : last_selected.x, last_selected.y);
						//cv::circle(frameBGR, new_point, 1, !dir ? cv::Scalar(255, 255, 100) : cv::Scalar(0, 20, 100), 10);
					}
					break;
				}
				else if (inner_start > 0 && x - inner_start > 10 && innerThresholded.ptr<uchar>(y)[dir ? end -x : x] == 0) {
					inner_end = x;
				}
				else if (outer_end > 0 && x - outer_end < 150 && inner_start < 0 && innerThresholded.ptr<uchar>(y)[dir ? end -x : x] == 255) {
					inner_start = x;
				}
				else if (outer_end < 0 && outer_start > 0 && x - outer_start > 10 && outerThresholded.ptr<uchar>(y)[dir ? end -x : x] == 0) {
					outer_end = x;
				}
				else if (outer_start < 0 && outerThresholded.ptr<uchar>(y)[dir ? end -x : x] == 255) {
					outer_start = x;
				}
			}
			if (break2) break;
		}
		if (points.size() > 3) {
			cv::Vec4f newLine;
			cv::fitLine(points, newLine, CV_DIST_L2, 0, 0.1, 0.1);
			drawLine(frameBGR, frameHSV, newLine,1, cv::Scalar(0, 255*(dir+0.3), 0));
		}
		//break;
	}


	/*
	cv::imshow("io", innerThresholded + outerThresholded);
	cv::imshow("i", innerThresholded);
	cv::imshow("o", outerThresholded);
	*/


}

ObjectPosition ObjectFinder::ConvertPixelToRealWorld(const cv::Point2i &point, const cv::Point2i &frame_size)
{
	if (point.y >= 0 && point.x >= 0 && point.y < frame_size.y && point.x < frame_size.x){//If there is no object found

	}
		

	const cv::Point2d center (frame_size.x / 2.0, frame_size.y / 2.0);
	//Calculating distance
	double angle = (Vfov * (point.y - center.y) / center.y) + CamAngleDev;
	double distance = CamHeight / tan(angle * PI / 180);
	//Calculating horizontal deviation
	double hor_space = tan(Hfov)*distance;
	double HorizontalDev = (hor_space * (point.x - center.x) / center.x);
	double Hor_angle = atan(HorizontalDev / distance)* 180/PI;
	/*
	if (Hor_angle > 0){
		Hor_angle = 360 - Hor_angle;
	}
	Hor_angle = abs(Hor_angle);
	*/
	return{ distance, HorizontalDev, Hor_angle };
}


void ObjectFinder::WriteInfoOnScreen(const ObjectPosition &info){
	cv::Mat infoWindow(100, 250, CV_8UC3, cv::Scalar::all(0));
	std::ostringstream oss;
	oss << "Distance :" << info.distance;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "Horizontal Dev :" << info.horizontalDev;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "Horizontal angle :" << info.horizontalAngle;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	cv::namedWindow("Info Window");
	cv::imshow("Info Window", infoWindow);
	return;
}


