#include "WebUI.h"


WebUI::WebUI(unsigned short port) //: wsServer(port, 2)
{
}


WebUI::~WebUI()
{
}


int WebUI::createButton(const std::string& bar_name, char shortcut, std::function<void()> const &)
{
	return 0;
}
int WebUI::Draw()
{
	return 0;
}
void WebUI::clearButtons()
{
}
void WebUI::ShowImage(const cv::Mat &image, bool main)
{
}
void WebUI::AddEventListener(IUIEventListener *pEventListener)
{
}
void WebUI::RemoveEventListener(IUIEventListener *pEventListener)
{
}
void WebUI::putText(const std::string &text, cv::Point pos, double fontScale, cv::Scalar color)
{
}
