//
// Session.cc
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "smb_session.h"


bool sm4ns3::Session::connect(boost::asio::ip::tcp::resolver::iterator& resolvIt) 
{
	boost::system::error_code ec;
	socket_.connect(*resolvIt, ec);
	return !ec;;	
}
	

bool sm4ns3::Session::isOpen() const 
{
	return socket_.is_open();
}

void sm4ns3::Session::closeSocket() 
{
	socket_.close();
}


bool sm4ns3::Session::write(const BundleHeader& header, std::string& t)
{
	// Format the header.
	outbound_header_ = BundleParser::make_bundle_header(header);
	if (outbound_header_.empty()) { return false; }

	// Write the serialized data to the socket. We use "gather-write" to send
	// both the header and the data in a single write operation.
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(outbound_header_));
	buffers.push_back(boost::asio::buffer(t));
	int res = boost::asio::write(socket_, buffers);
	return res;
}


bool sm4ns3::Session::read(std::string& t)
{
	boost::system::error_code ec;
	boost::asio::read(socket_, boost::asio::buffer(inbound_header_,header_length), ec);
	if(ec) {
		std::cout <<"synchronous Read error-0 [" << ec.message() << "]\n";
		return false;
	}

	unsigned int remLen = BundleParser::read_bundle_header(std::string(inbound_header_, header_length)).remLen;
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





