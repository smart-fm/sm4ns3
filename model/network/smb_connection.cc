/*
 * Connector.cpp
 *
 *  Created on: Jul 1, 2013
 *      Author: vahid
 */

//#include "ns3/simulator.h"
#include "smb_connection.h"
//#include "smb_broker.h"
#include "smb_serializer.h"

#include <boost/bind.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>
#include<jsoncpp/json/json.h>

using namespace boost;
//Macro used for callbacks
//#define CALL_MEMBER_FN(object, ptrToMember) ((*object).*(ptrToMember))
namespace sim_mob {
//boost::condition_variable cond_var;
//boost::mutex mutex_;
//void wait() {
//	boost::unique_lock<boost::mutex> lock(mutex_);
//	cond_var.wait(lock);
//}

Connection::Connection(boost::asio::io_service &io_service_/*, Broker* broker,
		messageReceiveCallback callback*/,boost::function<void(std::string)> messageReceiveCallback) :
		m_io_service(io_service_)/*, m_resolver(io_service_)*/
//		, theBroker(broker)
//		, receiveCallBack(callback)
		,m_messageReceiveCallback(messageReceiveCallback)
		{
//	NS_LOG_UNCOND( "Connection::Connection()" );

}

Connection::~Connection() {
	// TODO Auto-generated destructor stub
}

bool Connection::start() {
	return true;
}

bool Connection::connect(std::string host, std::string port)
{
	boost::asio::ip::tcp::resolver m_resolver(m_io_service);
	boost::asio::ip::tcp::resolver::query query(host, port);
	boost::system::error_code ec;
	boost::asio::ip::tcp::resolver::iterator it = m_resolver.resolve(query, ec);
	m_session.reset(new sim_mob::Session(m_io_service));
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
//	//expect to read WHOAREYOU message
//	bool res = m_session->read(incomingMessage, ec);
//	if (ec) {
//		NS_LOG_UNCOND( "read ec error" );
//		return false;
//	}
//	if (!res) {
//		NS_LOG_UNCOND( "read returned false" );
//		return false;
//	}
////	NS_LOG_UNCOND( "incoming: '" << incomingMessage << "'" );
//	msg_header messageHeader;
//	res = get_message_header(incomingMessage, messageHeader);
//	if (!res || messageHeader.msg_type != "WHOAREYOU") {
//		NS_LOG_UNCOND(
//				<< "Simmobility didn't send the WHOAREYOU message"
//				<< "[" << messageHeader.msg_type << "]" );
//		return false;
//	}
//	//write whoareyou message
//	std::string whoareyou = JsonParser::makeWhoAmIPacket();
//	m_session->write(whoareyou,ec);
//	if (ec) {
//		return false;
//	}
//	//expect to read READY message
//	m_session->read(incomingMessage, ec);
//	if (ec) {
//		return false;
//	}
//	res = get_message_header(incomingMessage, messageHeader);
//	if (!res || messageHeader.msg_type != "READY") {
//		NS_LOG_UNCOND(
//				<< "Simmobility didn't send the READY message"
//				);
//		return false;
//	}
//	async_receive();
	return true;
}
//void Connection::async_connect(std::string host, std::string port,
//		sim_mob::session_ptr &session_) {
//	boost::asio::ip::tcp::resolver m_resolver(m_io_service);
//	boost::asio::ip::tcp::resolver::query query(host, port);
//	boost::system::error_code ec;
//	boost::asio::ip::tcp::resolver::iterator it = m_resolver.resolve(query, ec);
//	m_session.reset(new sim_mob::Session(m_io_service));
//	//async version for reference, just in case
//	m_resolver.async_resolve(query,
//			boost::bind(&Connection::resolve_handler, this,
//					boost::asio::placeholders::error,
//					boost::asio::placeholders::iterator));
//}
//
//void Connection::resolve_handler(const boost::system::error_code &ec,
//		boost::asio::ip::tcp::resolver::iterator it) {
//	if (!ec) {
//		m_session->socket().async_connect(*it,
//				boost::bind(&Connection::connect_handler, this,
//						boost::asio::placeholders::error));
//	}
//}
//
//void Connection::connect_handler(const boost::system::error_code &ec) {
//	if (!ec) {
//		async_receive();
////		m_session->async_read(incomingMessage,
////				boost::bind(&Connection::initial_WHOAREYOU_handler, this,
////						boost::asio::placeholders::error));
//	}
//}



//void Connection::initial_WHOAREYOU_handler(
//		const boost::system::error_code &ec) {
//	msg_header messageHeader;
//	if (!ec && get_message_header(incomingMessage, messageHeader)) {
//		if (messageHeader.msg_type != "WHOAREYOU") {
//			NS_LOG_UNCOND(
//					<< "Simmobility didn't send the WHOAREYOU message, Trying Again"
//					);
//			m_session->async_read(incomingMessage,
//					boost::bind(&Connection::initial_WHOAREYOU_handler, this,
//							boost::asio::placeholders::error));
//			return;
//		}
//		m_session->async_read(incomingMessage,
//				boost::bind(&Connection::initial_READY_handler, this,
//						boost::asio::placeholders::error));
//	} else {
//		throw std::runtime_error(
//				"Error receiving the WHOAREYOU message from Simmobility");
//	}
//}
//
//void Connection::initial_READY_handler(const boost::system::error_code &ec) {
//	msg_header messageHeader;
//	if (!ec && get_message_header(incomingMessage, messageHeader)) {
//		if (messageHeader.msg_type != "READY") {
//			NS_LOG_UNCOND(
//					<< "Simmobility didn't send the Ready message, Trying Again"
//					);
//
//			m_session->async_read(incomingMessage,
//					boost::bind(&Connection::initial_READY_handler, this,
//							boost::asio::placeholders::error));
//			return;
//		}
//		read();
//	} else {
//		throw std::runtime_error(
//				"Error receiving the READY message from Simmobility");
//	}
//}

//reads into input
bool Connection::receive(std::string &input) {
	bool res = m_session->read(input);

	return res;
}
void Connection::async_receive() {
	if(!is_open())
	{
		NS_LOG_UNCOND( "Connection is down, reading will Fails " );
		return;
	}
	m_session->async_read(incomingMessage,
			boost::bind(&Connection::readHandler, this,
					boost::asio::placeholders::error));
}

void Connection::readHandler(const boost::system::error_code& e) {

	if (e) {
		NS_LOG_UNCOND( "Read Fail [" << e.message() << "]" );
		return;//todo is this ok?
	} else {
		//todo better alternative is to bind the broker's call back directly
//		CALL_MEMBER_FN(theBroker, receiveCallBack)(incomingMessage);
		m_messageReceiveCallback(incomingMessage);
	}
	async_receive();
}

bool Connection::send(std::string str) {
	return m_session->write(str);
}

void Connection::async_send(std::string str) {
//	m_session->async_write(str,
//			boost::bind(&Connection::sendHandler, this,
//					boost::asio::placeholders::error));
	m_session->write(str);
}
void Connection::sendHandler(const boost::system::error_code& e) {
}

bool Connection::is_open() {
	return m_session->socket().is_open();
}

} //sim_mob namespace
