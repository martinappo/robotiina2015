#include "ColorCalibrator.h"


ColorCalibrator::ColorCalibrator()
{
};

void ColorCalibrator::LoadImage(cv::Mat &image)
{
    this->image = image;
};
HSVColorRange ColorCalibrator::GetObjectThresholds (int index, const std::string &name)
{
	LoadConf(name);

	/*
    cvNamedWindow("ColorCalibrator", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    cvCreateTrackbar("LowH", "ColorCalibrator", &range.hue.low, 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "ColorCalibrator", &range.hue.high, 179);


    cvCreateTrackbar("LowS", "ColorCalibrator", &range.sat.low, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "ColorCalibrator", &range.sat.high, 255);


    cvCreateTrackbar("LowV", "ColorCalibrator", &range.val.low, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "ColorCalibrator", &range.val.high, 255);

    cv::Mat imgThresholded, imgHSV;
    cvtColor(image, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
	*/


    while (true)
    {
        //cv::inRange(imgHSV, cv::Scalar(range.hue.low, range.sat.low, range.val.low), cv::Scalar(range.hue.high, range.sat.high, range.val.high), imgThresholded); //Threshold the image

     //   cv::imshow(name.c_str(), imgThresholded); //show the thresholded image

        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
			cvDestroyWindow(name.c_str());
			std::cout << "esc key is pressed by user" << std::endl;
			SaveConf(name);
			return range;
		}
    }

};

ColorCalibrator::~ColorCalibrator(){
    cvDestroyWindow("ColorCalibrator");
}
