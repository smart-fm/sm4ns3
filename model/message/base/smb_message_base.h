//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)




/*
 * this file is inclusion place for custome Messgae classes
 * whose type will be used in defining templated handlers
 * This generic class is basically assumed to serve as a wrapper around a
 * data string with Json format.
 */

#pragma once

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <jsoncpp/json/json.h>

#include "smb_serializer.h"

namespace sim_mob
{
//Forward Declaration
class Handler;
typedef boost::shared_ptr<sim_mob::Handler> hdlr_ptr;

namespace comm
{
//Forward Declaration
class Message;
}

//This mess needs to go.
//typedef Json::Value msg_data_t;
//typedef sim_mob::comm::Message<msg_data_t> msg_t;
typedef boost::shared_ptr<sim_mob::comm::Message> msg_ptr;

namespace comm
{

//Base Message
class Message
{
	Json::Value data;
	msg_header header;
protected:
	void setData(const Json::Value& data_) {
		data = data_;
	}
public:
//	Message(){};
	Message(const Json::Value& data_, const msg_header& header):data(data_), header(header){}

	const Json::Value& getData() {
		return data;
	}

	const msg_header& getHeader() {
		return header;
	}
};
}//namespace comm
}//namespace sim_mob

