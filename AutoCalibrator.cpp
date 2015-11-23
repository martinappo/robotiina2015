#include "AutoCalibrator.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

AutoCalibrator::AutoCalibrator(ICamera * pCamera, IDisplay *pDisplay)
{
    range = {{0,179},{0,255},{0,255}};
	m_pCamera = pCamera;
	m_pDisplay = pDisplay;
	frame_size = m_pCamera->GetFrameSize();
	pDisplay->AddEventListener(this);
	screenshot_mode = LIVE_FEED;

//	reset();
};
void AutoCalibrator::LoadFrame()
{
	if(screenshot_mode != LIVE_FEED) return;
	frameBGR.copyTo(image);
	screenshot_mode = GRAB_FRAME;
};

HSVColorRange AutoCalibrator::GetObjectThresholds (int index, const std::string &name)
{
	clustered.copyTo(display);

	try {
		LoadConf(name);
	}
	catch (...) {
	}
	// order important, change name before state 
	this->object_name = name;
	screenshot_mode = GET_THRESHOLD;
	return range;


	/*
    cv::imshow(name.c_str(), image); //show the thresholded image
	cv::moveWindow(name.c_str(), 0, 0);
    cv::setMouseCallback(name.c_str(), [](int event, int x, int y, int flags, void* self) {
        if (event==cv::EVENT_LBUTTONUP) {
			((AutoCalibrator*)self)->mouseClicked(x, y, flags);
        }
		if (event == cv::EVENT_RBUTTONUP) {
			((AutoCalibrator*)self)->done = true;
		}
    }, this);


    done = false;
    while (!done)
    {
		std::this_thread::sleep_for(std::chrono::milliseconds(30)); // do not poll serial to fast
		
        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            std::cout << "esc key is pressed by user" << std::endl;
            done = true;
        }
	
    }
    cvDestroyWindow(name.c_str());
	*/
    SaveConf(object_name);
	screenshot_mode = THRESHOLDING;
    return range;

};
bool AutoCalibrator::OnMouseEvent(int event, float x, float y, int flags, bool bMainArea) {
	if (running && screenshot_mode == CROPPING && bMainArea){
		if (event == cv::EVENT_LBUTTONDOWN){
			thresholdCorner1 = cv::Point((int)(x), (int)(y));
			thresholdCorner2 = cv::Point((int)(x), (int)(y));
			drawRect = true;
		}
		else if (event == cv::EVENT_MOUSEMOVE && drawRect){
			thresholdCorner2 = cv::Point((int)(x), (int)(y));
		}
		else if (event == cv::EVENT_LBUTTONUP){
			if (cv::norm(thresholdCorner1 - thresholdCorner2) > 100){
				cv::Rect myROI(thresholdCorner1, thresholdCorner2);
				image = image(myROI);
				m_pDisplay->ShowImage(image);
				screenshot_mode = CALIBRATION;
				drawRect = false;
			}
			else{
				thresholdCorner1 = cv::Point(0, 0);
				thresholdCorner2 = cv::Point(0, 0);
				drawRect = false;
			}
		}
		return true;
	}else if (running && screenshot_mode == GET_THRESHOLD){
		if (event == cv::EVENT_LBUTTONUP && bMainArea) {
			mouseClicked((int)(x), (int)(y), flags);
		}
		if (event == cv::EVENT_RBUTTONUP) {
			SaveConf(this->object_name);
			screenshot_mode = THRESHOLDING;
		}
		return true;
	}
	return false;

};

void AutoCalibrator::mouseClicked(int x, int y, int flags) {
    cv::Mat imgHSV;
	cvtColor(image, imgHSV, CV_BGR2HSV);

    int label = bestLabels.at<int>(y*image.cols + x);
    //range =  {{179,0},{255,0},{255,0}} /* reverse initial values for min/max to work*/;
	std::vector<int> hue, sat, val;

    for(int i=0; i<image.cols*image.rows; i++) {
        if(bestLabels.at<int>(i) == label){
			hue.push_back(imgHSV.at<cv::Vec3b>(i).val[0]);
			sat.push_back(imgHSV.at<cv::Vec3b>(i).val[1]);
			val.push_back(imgHSV.at<cv::Vec3b>(i).val[2]);

        }
    }
	//get 5% and 95% percenties
	std::sort(hue.begin(), hue.end());
	std::sort(sat.begin(), sat.end());
	std::sort(val.begin(), val.end());

	int min_index = (int)(hue.size() * 0.05);
	int max_index = (int)(hue.size() * 0.95);

	if ((flags & cv::EVENT_FLAG_CTRLKEY)) {
		range.hue.low = std::min(range.hue.low, hue[min_index]);
		range.hue.high = std::max(range.hue.high, hue[max_index]);
		range.sat.low = std::min(range.sat.low, sat[min_index]);
		range.sat.high = std::max(range.sat.high, sat[max_index]);
		range.val.low = std::min(range.val.low, val[min_index]);
		range.val.high = std::max(range.val.high, val[max_index]);
	}
	else {
		range.hue.low = hue[min_index];
		range.hue.high = hue[max_index];
		range.sat.low = sat[min_index];
		range.sat.high = sat[max_index];
		range.val.low = val[min_index];
		range.val.high = val[max_index];
	}

	cv::Mat imgThresholded;
	cv::inRange(imgHSV, cv::Scalar(range.hue.low, range.sat.low, range.val.low), cv::Scalar(range.hue.high, range.sat.high, range.val.high), imgThresholded); //Threshold the image
	std::cout << cv::Scalar(range.hue.low, range.sat.low, range.val.low) << cv::Scalar(range.hue.high, range.sat.high, range.val.high) << std::endl;
	
	cv::Mat selected(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

	clustered.copyTo(selected, 255 - imgThresholded);
	selected.copyTo(display);

	//cv::imshow("auto thresholded", image); //show the thresholded image
    //cv::imshow("auto thresholded 2", imgThresholded); //show the thresholded image
    //cv::imshow("auto thresholded 3", clustered); //show the thresholded image

	//cv::imshow("original", image); //show the thresholded image
	//cv::imshow(this->object_name.c_str(), selected); //show the thresholded image

}
AutoCalibrator::~AutoCalibrator(){
	m_pDisplay->RemoveEventListener(this);
	WaitForStop();
}


void AutoCalibrator::Run() {
	while (!stop_thread)
	{
		boost::mutex::scoped_lock lock(mutex); //allow one command at a time
		if (screenshot_mode == LIVE_FEED){
			frameBGR = m_pCamera->Capture();
			frameBGR.copyTo(display);
			m_pDisplay->ShowImage(frameBGR);
		}
		else if (screenshot_mode == CROPPING){
			image.copyTo(display);
			if (drawRect && cv::norm(thresholdCorner1 - thresholdCorner2) > 100){
				line(display, thresholdCorner1, cv::Point(thresholdCorner2.x, thresholdCorner1.y), cv::Scalar(0, 0, 0), 1, 8, 0);
				line(display, thresholdCorner2, cv::Point(thresholdCorner2.x, thresholdCorner1.y), cv::Scalar(0, 0, 0), 1, 8, 0);

				line(display, thresholdCorner1, cv::Point(thresholdCorner1.x, thresholdCorner2.y), cv::Scalar(0, 0, 0), 1, 8, 0);
				line(display, thresholdCorner2, cv::Point(thresholdCorner1.x, thresholdCorner2.y), cv::Scalar(0, 0, 0), 1, 8, 0);
			}
			else{
				cv::putText(display, "Select area for thresholding", cv::Point(200, 220), cv::FONT_HERSHEY_DUPLEX, 0.9, cv::Scalar(23, 40, 245));
			}
			m_pDisplay->ShowImage(display);

		}
		else if (screenshot_mode == GRAB_FRAME){
			m_pDisplay->ShowImage(white);
			std::this_thread::sleep_for(std::chrono::milliseconds(150));
			frameBGR.copyTo(display);
			m_pDisplay->ShowImage(display);
			std::this_thread::sleep_for(std::chrono::milliseconds(1600));
			screenshot_mode = CROPPING;
		}
		else if (screenshot_mode == CALIBRATION){
			image.copyTo(display);
			cv::putText(display, "Please wait, clustering", cv::Point(200, 220), cv::FONT_HERSHEY_DUPLEX, 0.9, cv::Scalar(23, 40, 245));
			m_pDisplay->ShowImage(display);

			DetectThresholds(32);
			screenshot_mode = THRESHOLDING;
			//clustered.copyTo(display);
			m_pDisplay->ShowImage(display);
		}
		else if (screenshot_mode == THRESHOLDING){
			image.copyTo(display);
			m_pDisplay->ShowImage(display);
		}
		else if (screenshot_mode == GET_THRESHOLD) {
			cv::putText(display, object_name, cv::Point((int)(image.cols * 0.3), (int)(image.rows*0.3)), cv::FONT_HERSHEY_DUPLEX , 1, cv::Scalar(23, 67, 245));
			cv::putText(display, "(ctrl +) click to select pixels, right click back", cv::Point((int)(image.cols * 0.2), (int)(image.rows*0.5)), cv::FONT_HERSHEY_DUPLEX, 0.3, cv::Scalar(23, 67, 245));
			m_pDisplay->ShowImage(display);

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	}
}

void AutoCalibrator::DetectThresholds(int number_of_objects){
    cv::Mat img;
    //cvtColor(img,image,CV_BGR2HSV);
    image.copyTo(img);
    int origRows = img.rows;
    cv::Mat colVec = img.reshape(1, img.rows*img.cols); // change to a Nx3 column vector
    cv::Mat colVecD;
    int attempts = 1;

    double eps = 0.1;
    colVec.convertTo(colVecD, CV_32FC3, 1.0/255.0); // convert to floating point
    double compactness = cv::kmeans(colVecD, number_of_objects, bestLabels,
            cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, attempts, eps),
			attempts, cv::KMEANS_PP_CENTERS, centers);
    cv::Mat labelsImg = bestLabels.reshape(1, origRows); // single channel image of labels
    std::cout << "Compactness = " << compactness << std::endl;
    clustered = cv::Mat(1, img.rows*img.cols , CV_32FC3, 255);

    std::cout << centers << std::endl;

    //std::cout << ">" << " " << centers.at<cv::Point3f>(0) << " " << bestLabels.at<int>(3) << std::endl;
    std::cout << centers.at<float>(bestLabels.at<int>(0), 1) << std::endl;
    std::cout << img.cols*img.rows << ":" << bestLabels.rows << std::endl;
    for(int i=0; i<img.cols*img.rows; i++) {
		clustered.at<cv::Point3f>(i) = cv::Point3f(
				centers.at<float>(bestLabels.at<int>(i), 0),
				centers.at<float>(bestLabels.at<int>(i), 1),
				centers.at<float>(bestLabels.at<int>(i), 2)
			);
    }

    clustered.convertTo(clustered, CV_8UC3, 255);

	clustered = clustered.reshape(3, img.rows);


}
