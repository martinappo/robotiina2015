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
bool ObjectFinder::LocateCursor(cv::Mat &frameBGR, cv::Point2i cursor, OBJECT target, BallPosition &targetPos, RobotPosition robotPos){
	cv::Scalar color(0, 0, 0);
	cv::circle(frameBGR, cursor, 8, color, -1);
	targetPos.updateCoordinates(cursor, robotPos.fieldCoords, robotPos.getAngle());
	return true;
}


bool ObjectFinder::Locate(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, ObjectPosition &targetPos, RobotPosition robotPos) {
	cv::Point2i point = LocateOnScreen(HSVRanges, frameHSV, frameBGR, target);
	lastPosition = point;
	targetPos.updateCoordinates(point, robotPos.fieldCoords, robotPos.getAngle());
	return true;
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
	cv::line(img, startPoint, endPoint, color, thickness, 8, 0);
	return;
	

}
int ObjectFinder::IsolateField(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, bool detectBothBorders/* = false*/, bool nightVision/* = false*/) {


	std::vector<cv::Vec4i> lines;
	std::vector<cv::Vec4i> lines2;

	cv::Vec4f newLine1;
	cv::Vec4f newLine2;
	float distance = (float)(INT_MAX);
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
		return (int)(distance);
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

	return (int)(distance);


}

void ObjectFinder::IsolateFieldOld(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR) {

	cv::Mat innerThresholded = HSVRanges[INNER_BORDER];
	cv::Mat outerThresholded = HSVRanges[OUTER_BORDER];

	/*
	cv::Mat BLUE_GATEThresholded;
	inRange(frameHSV, cv::Scalar(BLUE_GATE.hue.low, BLUE_GATE.sat.low, BLUE_GATE.val.low), cv::Scalar(BLUE_GATE.hue.high, BLUE_GATE.sat.high, BLUE_GATE.val.high), BLUE_GATEThresholded); //Threshold the image
	cv::Mat YELLOW_GATEThresholded;
	inRange(frameHSV, cv::Scalar(YELLOW_GATE.hue.low, YELLOW_GATE.sat.low, YELLOW_GATE.val.low), cv::Scalar(YELLOW_GATE.hue.high, YELLOW_GATE.sat.high, YELLOW_GATE.val.high), YELLOW_GATEThresholded); //Threshold the image
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

//	cv::Mat gateThresholded = BLUE_GATEThresholded + YELLOW_GATEThresholded;

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

