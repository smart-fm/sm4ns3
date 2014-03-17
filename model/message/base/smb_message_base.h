/*
 * this file is inclusion place for custome Messgae classes
 * whose type will be used in defining templated handlers
 * This generic class is basically assumed to serve as a wrapper around a
 * data string with Json format.
 */

#pragma once

#include <iostream>
#include<boost/shared_ptr.hpp>
#include<jsoncpp/json/json.h>

namespace sim_mob
{
//Forward Declaration
class Handler;
typedef boost::shared_ptr<sim_mob::Handler> hdlr_ptr;

namespace comm
{
//Forward Declaration
template<class T>
class Message;
}

//todo do something here. the following std::string is spoiling messge's templatization benefits
typedef Json::Value msg_data_t;
typedef sim_mob::comm::Message<msg_data_t> msg_t;
typedef boost::shared_ptr<msg_t> msg_ptr; //putting std::string here is c++ limitation(old standard). don't blame me!-vahid

namespace comm
{

//Base Message
template<class T>
class Message
{
	T data;
	hdlr_ptr handler;
protected:
	void setData(T& data_)
	{
		data = data_;
	}
public:
	Message(){};
	Message(T data_):data(data_){}
	hdlr_ptr supplyHandler(){
		return handler;;
	}
	void setHandler( hdlr_ptr handler_)
	{
		handler = handler_;
	}
	T& getData()
	{
		return data;
	}
	virtual boost::shared_ptr<Message> clone(T&) = 0;
};
}//namespace comm
}//namespace sim_mob

