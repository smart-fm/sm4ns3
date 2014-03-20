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

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

namespace sm4ns3 {

/***
 * The Session class provides serialization primitives on top of a socket.
 * Each "bundle" sent using this class consists of:
 * @li A short "bundle header", which tells how much data is following (8 bytes, but varies in meaning between the old version and the new. old=length in hex).
 * @li The "bundle data" itself.
 */
class Session : public boost::enable_shared_from_this<Session> {
public:
	Session(boost::asio::io_service& io_service) : socket_(io_service)
	{
	}

	///Connect this Session's socket.
	bool connect(boost::asio::ip::tcp::resolver::iterator& resolvIt) 
	{
		boost::system::error_code ec;
		socket_.connect(*resolvIt, ec);
		return !ec;;	
	}
	
	///Is this Session's socket open?
	bool isOpen() const 
	{
		return socket_.is_open();
	}

	//Create a "version 0" (old-style) header, consisting of the data's length as an 8-byte hex (text) string.
	std::string make_bundle_header_v0(const std::string& data) 
	{
		std::ostringstream header_stream;
		header_stream << std::setw(header_length) << std::hex << data.size();
		if (!header_stream || header_stream.str().size() != header_length) {
			return "";
		}
		return header_stream.str();
	}

	//Read a "version 0" (old-style) header, returning the length of the remaining data section in bytes.
	unsigned int read_bundle_header_v0(const std::string& header) 
	{
		std::istringstream is(header);
		unsigned int res = 0;
		if (!(is >> std::hex >> res)) {
			return 0;
		}
		return res;
	}


	/// Synchronously write a data structure to the socket.
	bool write(std::string& t)
	{
		// Format the header.
		outbound_header_ = make_bundle_header_v0(t);
		if (outbound_header_.empty()) { return false; }

		// Write the serialized data to the socket. We use "gather-write" to send
		// both the header and the data in a single write operation.
		std::vector<boost::asio::const_buffer> buffers;
		buffers.push_back(boost::asio::buffer(outbound_header_));
		buffers.push_back(boost::asio::buffer(t));
		int res = boost::asio::write(socket_, buffers);
		return res;
	}

	/// Asynchronously write a data structure to the socket.
	template <typename Handler>
	void async_write(std::string & t, Handler handler)
	{
		// Format the header.
		outbound_header_ = make_bundle_header_v0(t);
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
	}

	/// synchronously read a data structure from the socket.
	bool read(std::string& t)
	{
		boost::system::error_code ec;
		boost::asio::read(socket_, boost::asio::buffer(inbound_header_,header_length), ec);
		if(ec) {
			std::cout <<"synchronous Read error-0 [" << ec.message() << "]\n";
			return false;
		}

		unsigned int remLen = read_bundle_header_v0(std::string(inbound_header_, header_length));
		if (remLen == 0) {
			return false;
		}

		// Start an asynchronous call to receive the data.
		inbound_data_.resize(remLen);
		boost::asio::read(socket_, boost::asio::buffer(inbound_data_,remLen), ec);
		if(ec) {
			std::cout <<"synchronous Read error-1 [" << ec.message() << "]\n";
			return false;
		}

		try {
			std::string archive_data(&inbound_data_[0], remLen);
			t = archive_data;
		} catch (std::exception& e) {
			std::cout <<"synchronous Read error-2 [" << e.what() << "]";
			return false;
		}
		return t.size();
	}


	/// Asynchronously read a data structure from the socket.
	template <typename Handler>
	void async_read(std::string& t, Handler handler) 
	{
		// Issue a read operation to read exactly the number of bytes in a header.
		void (Session::*f)(const boost::system::error_code&,std::string&, boost::tuple<Handler>)
			= &Session::handle_read_header<Handler>;
		boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
			boost::bind(f, shared_from_this(), boost::asio::placeholders::error, boost::ref(t), boost::make_tuple(handler)));
	}


	/// Handle a completed read of a message header. The handler is passed using
	/// a tuple since boost::bind seems to have trouble binding a function object
	/// created using boost::bind as a parameter.
	template <typename Handler>
	void handle_read_header(const boost::system::error_code& e, std::string& t, boost::tuple<Handler> handler)
	{
		if (e) {
			boost::get<0>(handler)(e);
		} else {
			// Determine the length of the serialized data.
			unsigned int remLen = read_bundle_header_v0(std::string(inbound_header_, header_length));
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

	/// Handle a completed read of message data.
	template <typename Handler>
	void handle_read_data(const boost::system::error_code& e, std::string& t, boost::tuple<Handler> handler)
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

private:
	/// The underlying socket.
	boost::asio::ip::tcp::socket socket_;

	/// The size of a fixed length header.
	enum { header_length = 8 };

	/// Holds an outbound header.
	std::string outbound_header_;

	/// Holds the outbound data.
	std::string outbound_data_;

	/// Holds an inbound header.
	char inbound_header_[header_length];

	/// Holds the inbound data.
	std::vector<char> inbound_data_;
};


}

