//
// Session.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/asio.hpp>

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include "bundle_version.h"

namespace sm4ns3 {

/***
 * The Session class provides serialization primitives on top of a socket.
 * Each "bundle" sent using this class consists of:
 * @li A short "bundle header", which tells how much data is following (8 bytes, but varies in meaning between the old version and the new. old=length in hex).
 * @li The "bundle data" itself.
 */
class Session : public boost::enable_shared_from_this<Session> {
public:
	Session(boost::asio::io_service& io_service) : socket_(io_service) {}

	///Connect this Session's socket.
	bool connect(boost::asio::ip::tcp::resolver::iterator& resolvIt);
	
	///Is this Session's socket open?
	bool isOpen() const;

	///Close this socket.
	void closeSocket();

	/// Synchronously write a data structure to the socket.
	bool write(const BundleHeader& header, std::string& t);

	/// synchronously read a data structure from the socket.
	bool read(std::string& t);

	///Asynchronously write a data structure to the socket.
	///TODO: Currently unused.
	/*template <typename Handler>
	void async_write(const BundleHeader& header, std::string & t, Handler handler);*/

	/// Asynchronously read a data structure from the socket.
	template <typename Handler>
	void async_read(std::string& t, Handler handler);


private:
	/// Handle a completed read of a message header. The handler is passed using
	/// a tuple since boost::bind seems to have trouble binding a function object
	/// created using boost::bind as a parameter.
	template <typename Handler>
	void handle_read_header(const boost::system::error_code& e, std::string& t, boost::tuple<Handler> handler);

	/// Handle a completed read of message data.
	template <typename Handler>
	void handle_read_data(const boost::system::error_code& e, std::string& t, boost::tuple<Handler> handler);


private:
	/// The underlying socket.
	boost::asio::ip::tcp::socket socket_;

	/// Holds an outbound header.
	std::string outbound_header_;

	/// Holds the outbound data.
	std::string outbound_data_;

	/// Holds an inbound header.
	char inbound_header_[header_length];

	/// Holds the inbound data.
	std::vector<char> inbound_data_;
};


} //End sm4ns3



/////////////////////////////////////////////////
// Template implementation
/////////////////////////////////////////////////


//TODO: Currently unused.
/*template <typename Handler>
void sm4ns3::Session::async_write(const BundleHeader& header, std::string & t, Handler handler)
{
	// Format the header.
	outbound_header_ = BundleParser::make_bundle_header(header, t);
	if (outbound_header_.empty()) {
		// Something went wrong, inform the caller.
		boost::system::error_code error(boost::asio::error::invalid_argument);
		socket_.get_io_service().post(boost::bind(handler, error));
		return;
	}

	// Write the serialized data to the socket. We use "gather-write" to send
	// both the header and the data in a single write operation.
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(outbound_header_));
	buffers.push_back(boost::asio::buffer(t));
	boost::asio::async_write(socket_, buffers, handler);
}*/


template <typename Handler>
void sm4ns3::Session::async_read(std::string& t, Handler handler) 
{
	// Issue a read operation to read exactly the number of bytes in a header.
	void (Session::*f)(const boost::system::error_code&,std::string&, boost::tuple<Handler>)
		= &Session::handle_read_header<Handler>;
	boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
		boost::bind(f, shared_from_this(), boost::asio::placeholders::error, boost::ref(t), boost::make_tuple(handler)));
}


template <typename Handler>
void sm4ns3::Session::handle_read_header(const boost::system::error_code& e, std::string& t, boost::tuple<Handler> handler)
{
	if (e) {
		boost::get<0>(handler)(e);
	} else {
		// Determine the length of the serialized data.
		unsigned int remLen = BundleParser::read_bundle_header(std::string(inbound_header_, header_length)).remLen;
		if (remLen == 0) {
			// Header doesn't seem to be valid. Inform the caller.
			boost::system::error_code error(boost::asio::error::invalid_argument);
			boost::get<0>(handler)(error);
			return;
		}

		// Start an asynchronous call to receive the data.
		inbound_data_.resize(remLen);
		void (Session::*f)(const boost::system::error_code&, std::string&, boost::tuple<Handler>)
			= &Session::handle_read_data<Handler>;
		boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
			boost::bind(f, shared_from_this(), boost::asio::placeholders::error, boost::ref(t), handler));
	}
}

template <typename Handler>
void sm4ns3::Session::handle_read_data(const boost::system::error_code& e, std::string& t, boost::tuple<Handler> handler)
{
	if (e) {
		boost::get<0>(handler)(e);
	} else {
		// Extract the data structure from the data just received.
		try {
			std::string archive_data(&inbound_data_[0], inbound_data_.size());
			t = archive_data;
		} catch (std::exception& e) {
			// Unable to decode data.
			boost::system::error_code error(boost::asio::error::invalid_argument);
			boost::get<0>(handler)(error);
			return;
		}

		// Inform caller that data has been received ok.
		boost::get<0>(handler)(e);
	}
}




