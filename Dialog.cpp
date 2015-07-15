#include "dialog.h"

Dialog::Dialog(const std::string &title, int flags/* = CV_WINDOW_AUTOSIZE*/) {

    m_title = title;
    int baseLine;
    m_buttonHeight = cv::getTextSize("Ajig6", cv::FONT_HERSHEY_DUPLEX, 0.9, 1, &baseLine).height * 2;

	cv::namedWindow(m_title, CV_WINDOW_FULLSCREEN);
	//	cvSetWindowProperty(m_title.c_str(), CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	cv::moveWindow(m_title, 0, 0);
	cv::setMouseCallback(m_title, [](int event, int x, int y, int flags, void* self) {
		((Dialog*)self)->mouseX = x;
		((Dialog*)self)->mouseY = y;
		if (event == cv::EVENT_LBUTTONUP){
			((Dialog*)self)->mouseClicked(x, y);
		}
	}, this);


};

int Dialog::createButton(const std::string& bar_name, std::function<void()> const & on_change){
	boost::mutex::scoped_lock lock(mutex); //allow one command at a time
	m_buttons.push_back(std::make_tuple(bar_name, on_change));
	return 0;
};

void Dialog::clearButtons() {
	boost::mutex::scoped_lock lock(mutex); //allow one command at a time
	m_buttons.clear();
}

int Dialog::show(const cv::Mat background) {
	cv::Mat image;
	background.copyTo(image);
    int window_width = image.cols;
    int window_height = image.rows;
    //cv::Mat image = cv::Mat::zeros( window_height, window_width, CV_8UC3 );

    int i = 0;
    for (const auto& button : m_buttons) {
        cv::putText(image, std::get<0>(button), cv::Point(30, (++i)*m_buttonHeight ), cv::FONT_HERSHEY_DUPLEX, 0.9, cv::Scalar(255,255,255));

    }
	cv::imshow(m_title, image);
	return 0;
};

void Dialog::mouseClicked(int x, int y) {
	boost::mutex::scoped_lock lock(mutex); //allow one command at a time
	int index = round((float)y / m_buttonHeight) - 1;
    if (index < m_buttons.size()){
        auto button = m_buttons[index];
		std::get<1>(button)();
        m_close = true;
    }

}
