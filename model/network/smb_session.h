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

#include "ns3/log.h"
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

namespace sim_mob {

/// The Session class provides serialization primitives on top of a socket.
/**
 * Each message sent using this class consists of:
 * @li An 8-byte header containing the length of the serialized data in
 * hexadecimal.
 * @li The serialized data.
 */
class Session;
class Session : public boost::enable_shared_from_this<Session>
{
public:
  /// Constructor.
  Session(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  /// Get the underlying socket. Used for making a Session or for accepting
  /// an incoming Session.
  boost::asio::ip::tcp::socket& socket()
  {
    return socket_;
  }

  /// Asynchronously write a data structure to the socket.
  bool write(std::string & t)
  {
    // Format the header.
    std::ostringstream header_stream;
    header_stream << std::setw(header_length)
      << std::hex << t.size();
    if (!header_stream || header_stream.str().size() != header_length)
    {
      return false;
    }
    outbound_header_ = header_stream.str();

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
    std::ostringstream header_stream;
    header_stream << std::setw(header_length)
      << std::hex << t.size();
    if (!header_stream || header_stream.str().size() != header_length)
    {
      // Something went wrong, inform the caller.
      boost::system::error_code error(boost::asio::error::invalid_argument);
      socket_.get_io_service().post(boost::bind(handler, error));
      return;
    }
    outbound_header_ = header_stream.str();

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
    if(ec)
    {
    	NS_LOG_UNCOND( "synchronous Read error-0 [" << ec.message() << "]" );
    }
    else
    {
//    	NS_LOG_UNCOND( "sync read-1 : " << nob << " bytes read" );
    }
    std::istringstream is(std::string(inbound_header_, header_length));
    std::size_t inbound_data_size = 0;
    if (!(is >> std::hex >> inbound_data_size))
    {
      return false;
    }
    // Start an asynchronous call to receive the data.
    inbound_data_.resize(inbound_data_size);
    boost::asio::read(socket_, boost::asio::buffer(inbound_data_,inbound_data_size), ec);
    if(ec)
    {
//    	NS_LOG_UNCOND( "synchronous Read error-1 [" << ec.message() << "]" );
    }
    else
    {
//    	NS_LOG_UNCOND( "sync read-2 : " << nob << " bytes read" );
    }
    try
    {
      std::string archive_data(&inbound_data_[0], inbound_data_size);
      t = archive_data;
    }
    catch (std::exception& e)
    {
    	NS_LOG_UNCOND( "synchronous Read error [" << e.what() << "]" );
      return false;
    }
    return t.size();
  }
  /// Asynchronously read a data structure from the socket.
  template <typename Handler>
  void async_read(std::string& t, Handler handler)
  {
    // Issue a read operation to read exactly the number of bytes in a header.
    void (Session::*f)(
        const boost::system::error_code&,
        std::string&, boost::tuple<Handler>)
      = &Session::handle_read_header<Handler>;
    boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
        boost::bind(f,
          shared_from_this(), boost::asio::placeholders::error, boost::ref(t),
          boost::make_tuple(handler)));
  }

  /// Handle a completed read of a message header. The handler is passed using
  /// a tuple since boost::bind seems to have trouble binding a function object
  /// created using boost::bind as a parameter.
  template <typename Handler>
  void handle_read_header(const boost::system::error_code& e,
		  std::string& t, boost::tuple<Handler> handler)
  {
    if (e)
    {
      boost::get<0>(handler)(e);
    }
    else
    {
      // Determine the length of the serialized data.
      std::istringstream is(std::string(inbound_header_, header_length));
      std::size_t inbound_data_size = 0;
      if (!(is >> std::hex >> inbound_data_size))
      {
        // Header doesn't seem to be valid. Inform the caller.
        boost::system::error_code error(boost::asio::error::invalid_argument);
        boost::get<0>(handler)(error);
        return;
      }

      // Start an asynchronous call to receive the data.
      inbound_data_.resize(inbound_data_size);
      void (Session::*f)(
          const boost::system::error_code&,
          std::string&, boost::tuple<Handler>)
        = &Session::handle_read_data<Handler>;
      boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
        boost::bind(f, shared_from_this(),
          boost::asio::placeholders::error, boost::ref(t), handler));
    }
  }

  /// Handle a completed read of message data.
  template <typename Handler>
  void handle_read_data(const boost::system::error_code& e,
		  std::string& t, boost::tuple<Handler> handler)
  {
    if (e)
    {
      boost::get<0>(handler)(e);
    }
    else
    {
      // Extract the data structure from the data just received.
      try
      {
        std::string archive_data(&inbound_data_[0], inbound_data_.size());
        t = archive_data;
      }
      catch (std::exception& e)
      {
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

typedef boost::shared_ptr<Session> session_ptr;

} // namespace sim_mob

