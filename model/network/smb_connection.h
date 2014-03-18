//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "smb_session.h"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace sim_mob {
//Forward Declaration
struct msg_header;
class BrokerBase;


class Connection {
private:
	BrokerBase* broker;
	boost::asio::io_service &m_io_service;
	sim_mob::session_ptr m_session;
	std::string incomingMessage;

public:
	Connection(boost::asio::io_service &io_service_, BrokerBase* broker);
	virtual ~Connection();

	bool start();

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
};

}

