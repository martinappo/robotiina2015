#pragma once
#include "types.h"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
class UdpServer
{
public:
	UdpServer(boost::asio::io_service &io, const boost::asio::ip::address & addr, unsigned short port, bool master);
	~UdpServer();
protected:
	void start_receive();

	void handle_receive(const boost::system::error_code& error,
		std::size_t /*bytes_transferred*/);

	void handle_send(boost::shared_ptr<std::string> /*message*/,
		const boost::system::error_code& /*error*/,
		std::size_t /*bytes_transferred*/);

	virtual void MessageReceived(const std::string & message){};
	void SendMessage(const std::string &message);
private:
	boost::asio::ip::udp::socket recv_socket;
	boost::asio::ip::udp::socket broadcast_socket;
	boost::asio::ip::udp::endpoint broadcast_endpoint;
	boost::asio::ip::udp::endpoint recv_endpoint;
	boost::array<char, 200> recv_buffer_;

};

