#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

namespace sm4ns3 {

/// The session class provides serialization primitives on top of a socket.
/**
 * Each message sent using this class consists of:
 * 8-byte header containing the length of the serialized data in hexadecimal.
 * and then The serialized data.
 */
class Session;
typedef boost::shared_ptr<Session> session_ptr;

class Session {
public:
	/// Constructor.
	Session(boost::asio::io_service &io_service) :
			socket_(io_service) {
	}

	~Session() {
		inbound_data_.clear();
	}

	/// Get the underlying socket. Used for making a Session or for accepting
	/// an incoming connection.
	boost::asio::ip::tcp::socket& socket() {
		return socket_;
	}

	bool makeWriteBuffer(std::string &input,
			std::vector<boost::asio::const_buffer> &output,
			boost::system::error_code &ec) {

		// Format the header.
		std::ostringstream header_stream;
		header_stream << std::setw(header_length) << std::hex << input.size();
		if (!header_stream || header_stream.str().size() != header_length) {
			// Something went wrong, inform the caller.
			ec = boost::system::error_code(
					boost::asio::error::invalid_argument);
			return false;
		}
		outbound_header_ = header_stream.str(); //not used

		output.push_back(boost::asio::buffer(outbound_header_));
		output.push_back(boost::asio::buffer(input));
		return true;
	}
	bool write(std::string &input, boost::system::error_code &ec) {

		std::vector<boost::asio::const_buffer> buffers;
		makeWriteBuffer(input, buffers, ec);
		if (ec) {
			return false;
		}
		boost::asio::write(socket_, buffers, ec);

//		boost::asio::write(socket_, boost::asio::buffer(input,input.size()), ec);
//		if(ec)
//		{
//			return false;
//		}
		return true;
	}
	/// Asynchronously write a data structure to the socket.
	template<typename Handler>
	void async_write(std::string &data, Handler handler) {
		boost::asio::async_write(socket_,
				boost::asio::buffer(/*outbound_data_*/data), handler);
	}

	bool read(std::string &input, boost::system::error_code &ec) {
		input.clear();
		/*size_t res =*/ boost::asio::read(socket_,
				boost::asio::buffer(inbound_header_, header_length), ec);
		if (ec) {
			return false;
		} else {
//			NS_LOG_UNCOND( "session read " << res << " bytes as header '";

		}
		std::istringstream is(std::string(inbound_header_, header_length));
//		NS_LOG_UNCOND( is.str() << "'" );
		std::size_t inbound_data_size = 0;
		is >> std::hex >> inbound_data_size;
		if (!(inbound_data_size)) {
			return false;
		}
		inbound_data_.resize(inbound_data_size);
		/*res =*/ boost::asio::read(socket_,
				boost::asio::buffer(inbound_data_, inbound_data_size), ec);
		if (ec) {
			return false;
		} else {
//			NS_LOG_UNCOND( "session read " << res << " bytes ' as data";
		}
		std::string received_data(&inbound_data_[0], inbound_data_.size());
//		NS_LOG_UNCOND( received_data << "'" );
		input = received_data;
		return true;
	}

	/// Asynchronously read a data structure from the socket.
	template<typename Handler>
	void async_read(std::string &input, Handler handler) {
		input.clear();
		// Issue a read operation to read exactly the number of bytes in a header.
		void (Session::*f)(const boost::system::error_code&,/*std::vector<char>*,*/
		std::string &,
				boost::tuple<Handler>) = &Session::handle_read_header<Handler>;
		boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
				boost::bind(f, this, boost::asio::placeholders::error,
						boost::ref(input)/*, t*/, boost::make_tuple(handler)));
	}

	/// Handle a completed read of a message header. The handler is passed using
	/// a tuple since boost::bind seems to have trouble binding a function object
	/// created using boost::bind as a parameter.
	template<typename Handler>
	void handle_read_header(const boost::system::error_code& e,/*std::vector<char>*t*/
	std::string &input, boost::tuple<Handler> handler) {
		if (e) {
			boost::get<0>(handler)(e);
		} else {
			std::istringstream is(std::string(inbound_header_, header_length));
//      std::cout  << "Inbound header is '" << is << "'" );
			std::size_t inbound_data_size = 0;
			is >> std::hex >> inbound_data_size;
//      is.clear();
//      is.str("");
			if (!(inbound_data_size)) {
				NS_LOG_UNCOND( "ERROR in session-Handle_read_header" );
				return;
			}
//			NS_LOG_UNCOND( "Inbound data size is '" << inbound_data_size << "'"
//					);
			inbound_data_.resize(inbound_data_size);

			void (Session::*f)(const boost::system::error_code&,/*std::vector<char>**/
			std::string &,
					boost::tuple<Handler>) = &Session::handle_read_data<Handler>;
			boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					boost::bind(f, this, boost::asio::placeholders::error, /*t,*/
					boost::ref(input), handler));
		}
	}

	/// Handle a completed read of message data.
	template<typename Handler>
	void handle_read_data(const boost::system::error_code& e,/*std::vector<char> * t*/
	std::string &input, boost::tuple<Handler> handler) {
		if (e) {
			boost::get<0>(handler)(e);
		} else {
			try {
				std::string archive_data(&inbound_data_[0],
						inbound_data_.size());
//				NS_LOG_UNCOND( "inbound_data_'" << archive_data << "'"
//						);
				input = archive_data;
				boost::get<0>(handler)(e);
			} catch (std::exception& e) {
				// Unable to decode data.
				NS_LOG_UNCOND( "Something wrong in the handle_read_data"
						);
				boost::system::error_code error(
						boost::asio::error::invalid_argument);
				boost::get<0>(handler)(error);
				return;
			}
		}

//		NS_LOG_UNCOND( "Clearing indound_data" );
		inbound_data_.clear();
	}

private:
	/// The underlying socket.
	boost::asio::ip::tcp::socket socket_;

	/// The size of a fixed length header.
	enum {
		header_length = 8
	};

	/// Holds an outbound header.
	std::string outbound_header_;

	/// Holds the outbound data.
	std::string outbound_data_;

	/// Holds an inbound header.
	char inbound_header_[header_length];

	/// Holds the inbound data.
	std::vector<char> inbound_data_;
};

} // namespace sm4ns3

