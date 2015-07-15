#include "kalmanFilter.h"

KalmanFilter::KalmanFilter(const cv::Point2i &startPoint){
	KF.transitionMatrix = *(cv::Mat_<float>(4, 4) << 1, 0, 1, 0, 
													 0, 1, 0, 1, 
													 0, 0, 1, 0, 
													 0, 0, 0, 1);

	
	measurement(0) = startPoint.x;
	measurement(1) = startPoint.y;
	estimated(0) = startPoint.x;
	estimated(1) = startPoint.y;

	KF.statePre.at<float>(0) = startPoint.x;
	KF.statePre.at<float>(1) = startPoint.y;
	KF.statePre.at<float>(2) = startPoint.x;
	KF.statePre.at<float>(3) = startPoint.y;

	KF.statePost.at<float>(0) = startPoint.x;
	KF.statePost.at<float>(1) = startPoint.y;
	KF.statePost.at<float>(2) = startPoint.x;
	KF.statePost.at<float>(3) = startPoint.y;


	setIdentity(KF.measurementMatrix);
	setIdentity(KF.processNoiseCov, cv::Scalar::all(0.005));
	setIdentity(KF.measurementNoiseCov, cv::Scalar::all(0.08));
	setIdentity(KF.errorCovPost, cv::Scalar::all(1));
}

cv::Point2i KalmanFilter::doFiltering(const cv::Point2i &point){
	// First predict, to update the internal statePre variable
	cv::Mat prediction = KF.predict();
	cv::Point predictPt(prediction.at<float>(0), prediction.at<float>(1));

	//Get point
	measurement(0) = point.x;
	measurement(1) = point.y;

	//The update phase
	estimated = KF.correct(measurement);
	cv::Point statePt(estimated.at<float>(0), estimated.at<float>(1));
	predictCount = 0;
	return statePt;
}

cv::Point2i KalmanFilter::getPrediction(){
	if (predictCount > 8) {
		return cv::Point(-1,-1); // stop predicting
	} else {
		cv::Mat prediction = KF.predict();
		cv::Point predictPt(prediction.at<float>(0), prediction.at<float>(1));
		predictCount++;
		return predictPt;	
	}
}

void KalmanFilter::reset(const cv::Point2i &startPoint){
	measurement(0) = startPoint.x;
	measurement(1) = startPoint.y;
	estimated(0) = startPoint.x;
	estimated(1) = startPoint.y;

	KF.statePre.at<float>(0) = startPoint.x;
	KF.statePre.at<float>(1) = startPoint.y;
	KF.statePre.at<float>(2) = startPoint.x;
	KF.statePre.at<float>(3) = startPoint.y;

	KF.statePost.at<float>(0) = startPoint.x;
	KF.statePost.at<float>(1) = startPoint.y;
	KF.statePost.at<float>(2) = startPoint.x;
	KF.statePost.at<float>(3) = startPoint.y;
}

