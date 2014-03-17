/*
 * Connection.h
 *
 *  Created on: Jul 1, 2013
 *      Author: vahid
 */

#pragma once

#include "smb_session.h"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace sim_mob {
//Forward Declaration
//class Broker;
struct msg_header;
class Connection {
private:
	boost::asio::io_service &m_io_service;
//	boost::asio::ip::tcp::resolver m_resolver;
	sim_mob::session_ptr m_session;
	std::string incomingMessage;
//	Broker *theBroker;
//	typedef void (Broker::*messageReceiveCallback)(std::string);
//	messageReceiveCallback receiveCallBack;
	boost::function<void(std::string)> m_messageReceiveCallback;
//	//connects in async mode
//	void resolve_handler(const boost::system::error_code &ec,
//			boost::asio::ip::tcp::resolver::iterator it);
//	//issues an async_read to continue connection process with whoareyou and ready message
//	void connect_handler(const boost::system::error_code &ec);
	//part of async connection that responds to WHOAREYOU query
//	void initial_WHOAREYOU_handler(const boost::system::error_code &ec);
//	//part of async connection that gets READY (confirmation from simmobility) + issue an async_read
//	void initial_READY_handler(const boost::system::error_code &ec);

public:
	Connection(boost::asio::io_service &io_service_/*, Broker* broker,
			messageReceiveCallback callback*/,
			boost::function<void(std::string)> messageReceiveCallback_);
	bool start();
//	void async_connect(std::string host, std::string port,
//			sim_mob::session_ptr &session_);
//	void resolve_handler(const boost::system::error_code &ec,
//					boost::asio::ip::tcp::resolver::iterator it);
//	void connect_handler(const boost::system::error_code &ec);
	//resolves, connects, authenticates, and issues an async_read
	bool connect(std::string host, std::string port);
	//	issues a read
	bool receive(std::string&);
	//isues an async_read
	void async_receive();
	//read handler of the async_read
	void readHandler(const boost::system::error_code& e);
	//issues a write
	bool send(std::string str);
	//issues an async_write --todo:not working
	void async_send(std::string str);
	//handler for async_write
	void sendHandler(const boost::system::error_code& e);
	//is the socket still working?
	bool is_open();
	virtual ~Connection();
};
} //sim_mob namespace

