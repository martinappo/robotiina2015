#pragma once
#include "types.h"
#include "Simple-WebSocket-Server\server_ws.hpp"

class WebUI :
	public IDisplay
{
protected:
	SimpleWeb::SocketServer<SimpleWeb::WS> wsServer;
public:
	WebUI(unsigned short port);
	virtual ~WebUI();
	virtual int createButton(const std::string& bar_name, char shortcut, std::function<void()> const &);
	virtual int Draw();
	virtual void clearButtons();
	virtual void ShowImage(const cv::Mat &image, bool main = true);
	virtual void AddEventListener(IUIEventListener *pEventListener);
	virtual void RemoveEventListener(IUIEventListener *pEventListener);
	virtual void putText(const std::string &text, cv::Point pos, double fontScale, cv::Scalar color);

};

