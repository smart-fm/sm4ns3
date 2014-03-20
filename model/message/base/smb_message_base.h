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

namespace sm4ns3
{

//New base message class; contains everything a Message is guranteed to have. 
//Does NOT support dynamic inheritance; never store a subclass using this class as a pointer.
struct MessageBase {
	std::string sender_id;   ///<Who sent this message.
	std::string sender_type; ///<The "type" of this sender (to be removed).
	std::string msg_type;    ///<The "type" of this message. Used to identify the subclass.
	std::string msg_cat;     ///<The "category" of this message (to be removed).
};




//Forward Declaration
class Handler;
typedef boost::shared_ptr<sm4ns3::Handler> hdlr_ptr;

namespace comm
{
//Forward Declaration
class Message;
}

//This mess needs to go.
//typedef Json::Value msg_data_t;
//typedef sm4ns3::comm::Message<msg_data_t> msg_t;
typedef boost::shared_ptr<sm4ns3::comm::Message> msg_ptr;

namespace comm
{

//Base Message
class Message
{
	Json::Value data;
	//msg_header header;
protected:
	void setData(const Json::Value& data_) {
		data = data_;
	}
public:
//	Message(){};
//	Message(const Json::Value& data_, const msg_header& header):data(data_), header(header){}

	const Json::Value& getData() {
		return data;
	}

/*	const msg_header& getHeader() {
		return header;
	}*/
};


}//namespace comm
}//namespace sm4ns3

