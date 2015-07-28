#pragma once
#include "types.h"
#include <functional>
#include <boost/thread/mutex.hpp>
#include <atomic>
/*
* No time to add QT support to OpenCV, have to make our own buttons (and dialogs)
* */

class Dialog: public IDisplay {
public:
    Dialog(const std::string &m_Title, int flags = CV_WINDOW_AUTOSIZE);
	int createButton(const std::string& bar_name, std::function<void()> const &);
    int show(const cv::Mat background);
	void clearButtons();
	virtual void ShowImage(const cv::Mat image);
	void ClearDisplay();

protected:
    void mouseClicked(int x, int y);
	std::atomic_int mouseX;
	std::atomic_int mouseY;

	cv::Mat display_empty;// (frameBGR.rows + 160, frameBGR.cols + 200, frameBGR.type(), cv::Scalar(0));
	cv::Mat display;// (frameBGR.rows + 160, frameBGR.cols + 200, frameBGR.type(), cv::Scalar(0));
	cv::Mat display_roi;// = display(cv::Rect(0, 0, frameBGR.cols, frameBGR.rows)); // region of interest
	cv::Mat cam_area;

private:
    bool m_close = false;
    std::string m_title;
	std::vector<std::tuple<std::string, std::function<void()>>> m_buttons;
    int m_buttonHeight = 60; /* calculated automatically*/
	boost::mutex click_mutex;
	boost::mutex display_mutex;

};
