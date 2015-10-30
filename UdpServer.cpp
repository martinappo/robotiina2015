#include "UdpServer.h"
extern boost::asio::ip::address bind_addr;
extern boost::asio::ip::address brdc_addr;


UdpServer::UdpServer(boost::asio::io_service &io, unsigned short port, bool master)
	: recv_socket(io, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port + !master))
	, broadcast_socket(io, boost::asio::ip::udp::endpoint(bind_addr, 0))
	, recv_endpoint(boost::asio::ip::udp::v4(), port + !master)
	, broadcast_endpoint(brdc_addr/*boost::asio::ip::address_v4::broadcast()*/, port + master)

{
	broadcast_socket.set_option(boost::asio::socket_base::broadcast(true));
	start_receive();
}


UdpServer::~UdpServer()
{
}

void UdpServer::start_receive()
{
	recv_socket.async_receive_from(
		boost::asio::buffer(recv_buffer_), recv_endpoint,
		boost::bind(&UdpServer::handle_receive, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void UdpServer::handle_receive(const boost::system::error_code& error,
	std::size_t /*bytes_transferred*/)
{
	//std::cout << "handle_receive" << std::endl;
	if (!error || error == boost::asio::error::message_size)
	{
		std::string message = std::string(recv_buffer_.begin(), recv_buffer_.end());
		//std::cout << "udp packet:" << message << std::endl;
		MessageReceived(message);
		/*
		boost::shared_ptr<std::string> message(
		new std::string(make_daytime_string()));

		socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
		boost::bind(&udp_server::handle_send, this, message,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
		*/
		start_receive();
	}
}

void UdpServer::handle_send(boost::shared_ptr<std::string> /*message*/,
	const boost::system::error_code& /*error*/,
	std::size_t /*bytes_transferred*/)
{
}

void UdpServer::SendMessage(const std::string &message){
	//std::cout << "SendMessage" << message << std::endl;

	boost::shared_ptr<std::string> x(
		new std::string(message));
	broadcast_socket.async_send_to(boost::asio::buffer(*x), broadcast_endpoint,
		boost::bind(&UdpServer::handle_send, this, x,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}
