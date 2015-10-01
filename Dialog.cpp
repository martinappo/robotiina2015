#include "Dialog.h"
/*
#define WINDOW_WIDTH 976
#define WINDOW_HEIGHT 840

#define CAM_WIDTH 720
#define CAM_HEIGHT 720
*/

Dialog::Dialog(const std::string &title, const cv::Size &ptWindowSize, const cv::Size &ptCamSize, int flags/* = CV_WINDOW_AUTOSIZE*/)
	: windowSize(ptWindowSize), camSize(ptCamSize)
{
	cv::Size windowSizeDefault = cv::Size((int)((double)camSize.width / 0.7), (int)((double)camSize.height / 0.7));

	if (windowSize != cv::Size(0, 0)) {
		double scale = (double)windowSize.width / (double)windowSizeDefault.width;
		camSize = cv::Size((int)((double)camSize.width * scale), (int)((double)camSize.height * scale));
	} 
	else {
		windowSize = cv::Size((int)((double)camSize.width / 0.7), (int)((double)camSize.width / 0.7));
	}
	fontScale = (double)windowSize.height / 1024;
    m_title = title;
	m_bMainCamEnabled = true;
    int baseLine;

	m_buttonHeight = cv::getTextSize("Ajig6", cv::FONT_HERSHEY_DUPLEX, fontScale, 1, &baseLine).height * 2;

	/*
    m_buttonHeight = cv::getTextSize("Ajig6", cv::FONT_HERSHEY_DUPLEX, fontScale, 1, &baseLine).height * 2;
	
	cv::namedWindow(m_title, CV_WINDOW_AUTOSIZE);
	//cvSetWindowProperty(m_title.c_str(), CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	cv::moveWindow(m_title, 0, 0);
	cv::setMouseCallback(m_title, [](int event, int x, int y, int flags, void* self) {
		for (auto pListener : ((Dialog*)self)->m_EventListeners){
			if (pListener->OnMouseEvent(event, (float)x / ((Dialog*)self)->camSize.x, (float)y / ((Dialog*)self)->camSize.y, flags)) {
				return; // event was handled
			}
		}

		((Dialog*)self)->mouseX = x;
		((Dialog*)self)->mouseY = y;
		if (event == cv::EVENT_LBUTTONUP){
			((Dialog*)self)->mouseClicked(x, y);
		}
	}, this);
*/

	display_empty = cv::Mat(windowSize, CV_8UC3, cv::Scalar(0));
	display = cv::Mat(windowSize, CV_8UC3, cv::Scalar(0));
	cam1_roi = display(cv::Rect(0, 0, camSize.width, camSize.height)); // region of interest
	int cam2_width = windowSize.width - camSize.width;
	cam2_roi = display(cv::Rect(windowSize - cv::Size(cam2_width, cam2_width), cv::Size(cam2_width, cam2_width)));
	//cam_area = cv::Mat(CAM_HEIGHT, CAM_WIDTH, CV_8UC3);
	Start();

};
Dialog::~Dialog(){
	WaitForStop();
}
void Dialog::ShowImage(const cv::Mat &image, bool main) {
	if (!m_bMainCamEnabled && main) return;
	boost::mutex::scoped_lock lock(display_mutex); //allow one command at a time
	image.copyTo(main ? cam1_area : cam2_area);
	//	resize(image, cam_area, cv::Size(CAM_WIDTH, CAM_HEIGHT));//resize image
	//resize(image, display, cv::Size(WINDOW_WIDTH, WINDOW_HEIGHT));//resize image
}

int Dialog::createButton(const std::string& bar_name, char shortcut, std::function<void()> const & on_change){
	boost::mutex::scoped_lock lock(click_mutex); //allow one command at a time
	m_buttons.push_back(std::make_tuple(bar_name, shortcut, on_change));
	return 0;
};

void Dialog::clearButtons() {
	boost::mutex::scoped_lock lock(click_mutex); //allow one command at a time
	m_buttons.clear();
}

void Dialog::ClearDisplay() {
//	boost::mutex::scoped_lock lock(mutex); //allow one command at a time
//	display_empty.copyTo(display);
}
void Dialog::putText(const std::string &text, cv::Point pos, double fontScale, cv::Scalar color) {
	boost::mutex::scoped_lock lock(click_mutex); //allow one command at a time

	if (pos.x < 0) pos.x = display.size().width + pos.x;
	if (pos.y < 0) pos.y = display.size().height + pos.y;
	std::string key = std::to_string(pos.x) + "_" + std::to_string(pos.y);
	m_texts[key] = std::make_tuple(pos, text, fontScale, color);
	//cv::putText(display, text, pos, cv::FONT_HERSHEY_DUPLEX, fontScale, color);
}


int Dialog::Draw() {
	boost::mutex::scoped_lock lock(click_mutex); //allow one command at a time
	//cv::Mat image;
	//background.copyTo(image);
    //int window_width = image.cols;
    //int window_height = image.rows;
    //cv::Mat image = cv::Mat::zeros( window_height, window_width, CV_8UC3 );
	//cam_area.copyTo(display_roi);
	cv::Mat main_roi = m_bCam1Active ? cam1_roi : cam2_roi;
	cv::Mat sec_roi = !m_bCam1Active ? cam1_roi : cam2_roi;
	if (m_bMainCamEnabled && cam1_area.size().height > 0) {
		resize(cam1_area, main_roi, main_roi.size());//resize image
	}
	if (cam2_area.size().height > 0) {
		resize(cam2_area, sec_roi, sec_roi.size());//resize image
	}

    int i = 0;
    for (const auto& button : m_buttons) {
		cv::putText(display, std::get<0>(button), cv::Point(30, (++i)*m_buttonHeight), cv::FONT_HERSHEY_DUPLEX, fontScale, cv::Scalar(255, 255, 255));

    }
	for (const auto& text : m_texts) {
		cv::putText(display, std::get<1>(text.second), std::get<0>(text.second), cv::FONT_HERSHEY_DUPLEX, std::get<2>(text.second), std::get<3>(text.second));

	}

	cv::imshow(m_title, display);
	display_empty.copyTo(display);

	return 0;
};
void Dialog::KeyPressed(int key){

	if (key == '-') return;
	boost::mutex::scoped_lock lock(click_mutex); //allow one command at a time
	for (auto btn : m_buttons){
		if(std::get<1>(btn) == key){
			std::get<2>(btn)();
			return;
		}
	}
}

void Dialog::mouseClicked(int x, int y) {
	boost::mutex::scoped_lock lock(click_mutex); //allow one command at a time
	unsigned int index = (int)(round((float)y / m_buttonHeight) - 1);
    if (index < m_buttons.size()){
        auto button = m_buttons[index];
		std::get<2>(button)();
        m_close = true;
    }

}
void Dialog::Run(){
	cv::namedWindow(m_title, CV_WINDOW_AUTOSIZE);
	//cvSetWindowProperty(m_title.c_str(), CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	cv::moveWindow(m_title, 0, 0);
	cv::setMouseCallback(m_title, [](int event, int x, int y, int flags, void* self) {
		bool bMainArea = x < ((Dialog*)self)->camSize.width && y < ((Dialog*)self)->camSize.height;
		cv::Point scaled;
		cv::Point offset;
		cv::Size size;
		cv::Size target;

		if (bMainArea) {
			((Dialog*)self)->cam1_roi.locateROI(size, offset);
			size = ((Dialog*)self)->cam1_roi.size();
			target = ((Dialog*)self)->cam1_area.size();
		}
		else {
			((Dialog*)self)->cam2_roi.locateROI(size, offset);
			size = ((Dialog*)self)->cam2_roi.size();
			target = ((Dialog*)self)->cam2_area.size();
		}
		scaled.x = (int)((double)(x - offset.x) / size.width * target.width);
		scaled.y = (int)((double)(y - offset.y) / size.height * target.height);
		for (auto pListener : ((Dialog*)self)->m_EventListeners){
			if (pListener->OnMouseEvent(event, (float)(scaled.x), (float)(scaled.y), flags, bMainArea)) {
				return; // event was handled
			}
		}

		((Dialog*)self)->mouseX = x;
		((Dialog*)self)->mouseY = y;
		if (event == cv::EVENT_LBUTTONUP){
			((Dialog*)self)->mouseClicked(x, y);
		}
	}, this);

	while (!stop_thread) {
		try {
			Draw();
			int key = cv::waitKey(10);
			KeyPressed(key);
		}
		catch (std::exception &e) {
			std::cout << "Dialog::Run, error: " << e.what() << std::endl;
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	cv::waitKey(0);
}
