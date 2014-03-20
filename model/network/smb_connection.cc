//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_connection.h"

#include <boost/bind.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>
#include <jsoncpp/json/json.h>

#include "smb_serializer.h"
#include "smb_broker.h"

using namespace boost;

sm4ns3::Connection::Connection(boost::asio::io_service &io_service_, BrokerBase* broker) :
	broker(broker),
	m_io_service(io_service_)
{
	if (!broker) { throw std::runtime_error("Null Broker in Connection."); }
}

sm4ns3::Connection::~Connection() 
{
}

bool sm4ns3::Connection::start() 
{
	return true;
}

bool sm4ns3::Connection::connect(std::string host, std::string port)
{
	boost::asio::ip::tcp::resolver m_resolver(m_io_service);
	boost::asio::ip::tcp::resolver::query query(host, port);
	boost::system::error_code ec;
	boost::asio::ip::tcp::resolver::iterator it = m_resolver.resolve(query, ec);
	m_session.reset(new sm4ns3::Session(m_io_service));

	//sync version for reference, just in case
	m_resolver.resolve(query,ec);
	if (ec) {
		return false;
	}
	m_session->socket().connect(*it,ec);
	if (ec) {
		NS_LOG_UNCOND( "connect ec error" );
		return false;
	}
	NS_LOG_UNCOND( "Connected" );
	return true;
}


//reads into input
bool sm4ns3::Connection::receive(std::string &input) 
{
	bool res = m_session->read(input);

	return res;
}

void sm4ns3::Connection::async_receive() 
{
	if(!is_open())
	{
		NS_LOG_UNCOND( "Connection is down, reading will Fails " );
		return;
	}
	m_session->async_read(incomingMessage,
			boost::bind(&Connection::readHandler, this,
					boost::asio::placeholders::error));
}

void sm4ns3::Connection::readHandler(const boost::system::error_code& e) 
{
	if (e) {
		NS_LOG_UNCOND( "Read Fail [" << e.message() << "]" );
		return;//todo is this ok?
	} else {
		//todo better alternative is to bind the broker's call back directly
		broker->onMessageReceived(incomingMessage);
	}
	async_receive();
}

bool sm4ns3::Connection::send(std::string str) 
{
	return m_session->write(str);
}

void sm4ns3::Connection::async_send(std::string str) 
{
	m_session->write(str);
}

void sm4ns3::Connection::sendHandler(const boost::system::error_code& e) 
{
}

bool sm4ns3::Connection::is_open() 
{
	return m_session->socket().is_open();
}


