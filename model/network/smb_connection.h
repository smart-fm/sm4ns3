//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>

#include "smb_session.h"
#include "bundle_version.h"

namespace sm4ns3 {

//Forward Declaration
class BrokerBase;


class Connection {
public:
	Connection(boost::asio::io_service &io_service_, BrokerBase* broker, const std::string& simmob_host, const std::string& simmob_port);

	//resolves, connects, authenticates, and issues an async_read. 
	bool connect();

	//	issues a read
	bool receive(std::string&);

	//isues an async_read
	void async_receive();

	//issues a write
	bool send(const BundleHeader& header, std::string str);

	//is the socket still working?
	bool isOpen();

protected:
	//read handler of the async_read
	void readHandler(const boost::system::error_code& e);


private:
	BrokerBase* broker;
	boost::asio::io_service &m_io_service;
	boost::shared_ptr<Session> m_session;
	std::string incomingMessage;

	std::string simmob_host;
	std::string simmob_port;

};

}

