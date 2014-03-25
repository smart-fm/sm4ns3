//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_connection.h"

#include <boost/bind.hpp>

#include "smb_broker.h"


sm4ns3::Connection::Connection(boost::asio::io_service &io_service_, BrokerBase* broker, const std::string& simmob_host, const std::string& simmob_port) :
	broker(broker), m_io_service(io_service_), simmob_host(simmob_host), simmob_port(simmob_port)
{
	if (!broker) { throw std::runtime_error("Null Broker in Connection."); }
}

bool sm4ns3::Connection::connect()
{
	if (!m_session) {
		m_session.reset(new sm4ns3::Session(m_io_service));
	}

	boost::asio::ip::tcp::resolver m_resolver(m_io_service);
	boost::asio::ip::tcp::resolver::query query(simmob_host, simmob_port);
	
	boost::system::error_code ec;
	boost::asio::ip::tcp::resolver::iterator it = m_resolver.resolve(query, ec);

	//sync version for reference, just in case
	m_resolver.resolve(query,ec);
	if (ec) {
		throw std::runtime_error("Couldn't resolve server.");
	}

	return m_session->connect(it);
}


bool sm4ns3::Connection::receive(BundleHeader& header, std::string &input)
{
	return m_session->read(header, input);
}

void sm4ns3::Connection::async_receive() 
{
	if(!isOpen()) { throw std::runtime_error("Connection is down, reading will Fail"); }

	incomingMessage = "";
	m_session->async_read(incomingHeader, incomingMessage, boost::bind(&Connection::readHandler, this, boost::asio::placeholders::error));
}

void sm4ns3::Connection::readHandler(const boost::system::error_code& e) 
{
	if (e) {
		std::cout <<"Read Failed with error: " << e.message() <<"\n";
		if(e.message() == "End of file") {
			//We can handle this; no need to return.
			std::cout <<"EOF on incomingMessage= '" << incomingMessage << "'\n";
			m_session->closeSocket(); 
		} else {
			return;
		}
	}

	broker->onMessageReceived(incomingHeader, incomingMessage);
	async_receive();
}

bool sm4ns3::Connection::send(const BundleHeader& header, std::string str) 
{
	return m_session->write(header, str);
}


bool sm4ns3::Connection::isOpen() 
{
	return m_session->isOpen();
}


