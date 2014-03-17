//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_rr_factory.h"
#include "smb_rr_casts.h"

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include<jsoncpp/json/json.h>
#include <stdexcept>

namespace sim_mob {
namespace roadrunner {

RR_Factory::RR_Factory() {
	// TODO Auto-generated constructor stub
	MessageMap = boost::assign::map_list_of
//			("READY" , READY)
			("MULTICAST", MULTICAST)
			("UNICAST", UNICAST)
			("ALL_LOCATIONS_DATA", ALL_LOCATIONS_DATA)
			("READY_TO_RECEIVE",READY_TO_RECEIVE)
			("AGENTS_INFO", AGENTS_INFO);

	registerMessage(std::string("MULTICAST"), msg_ptr (new MSG_MULTICAST()));
	registerMessage(std::string("UNICAST"), msg_ptr (new MSG_UNICAST()));
}

RR_Factory::~RR_Factory() {}
//gets a handler either from a chche or by creating a new one
hdlr_ptr  RR_Factory::getHandler(MessageType type){
	hdlr_ptr handler;
	//if handler is already registered && the registered handler is not null
	std::map<MessageType, hdlr_ptr >::iterator it = HandlerMap.find(type);
	if((it != HandlerMap.end())&&((*it).second!= 0))
	{
		//get the handler ...
		handler = (*it).second;
	}
	else
	{
		//else, create a cache entry ...
		bool typeFound = true;
		switch(type)
		{
		case MULTICAST:
			handler.reset(new sim_mob::roadrunner::HDL_MULTICAST());
			break;
		case UNICAST:
			handler.reset(new sim_mob::roadrunner::HDL_UNICAST());
			break;
		default:
			NS_LOG_UNCOND( "Couldn't find a handler" );
			typeFound = false;
		}
		//register this baby
		if(typeFound)
		{
			HandlerMap[type] = handler;
		}
	}

	return handler;
}

bool RR_Factory::createMessage(msg_data_t &input, msg_ptr& output)
{
	msg_header messageHeader;
	if (!sim_mob::JsonParser::parseMessageHeader(input, messageHeader)) {
		NS_LOG_UNCOND( "RR_Factory::createMessage=>parseMessageHeader Failed 1" );
		return false;
	}
	bool res;
	const msg_ptr prototype = getPrototype(messageHeader.msg_type, res);
	if(!res)
	{
		NS_LOG_UNCOND( "RR_Factory::createMessage=>parseMessageHeader Failed 2" );
		return false;
	}
	output = prototype->clone(input);
	output->setHandler(getHandler(MessageMap[messageHeader.msg_type]));

	return true;
}
//creates a message with correct format + assigns correct handler
//todo improve the function to handle array of messages stored in the input string
 bool RR_Factory::createMessage(std::string &input, std::vector<msg_ptr>& output)
{
//	std::vector<msg_t> result;
	std::string type, data;
	Json::Value root;
	sim_mob::pckt_header packetHeader;
	if(!sim_mob::JsonParser::parsePacketHeader(input, packetHeader, root))
	{
		return false;
	}
	if(!sim_mob::JsonParser::getPacketMessages(input,root))
	{
		return false;
	}
//	NS_LOG_UNCOND( "nof_messages = " << root.size() );
	for (unsigned int index = 0; index < root.size(); index++) {
		msg_header messageHeader;
//		std::string  msgStr;// =  /*const_cast<std::string&>*/(root[index].asString());
		if (!sim_mob::JsonParser::parseMessageHeader(root[index], messageHeader)) {
			continue;
		}
		msg_data_t & curr_json = root[index];
//		NS_LOG_UNCOND( index << ": (" <<  messageHeader.msg_type << ")" );
		msg_ptr msg = msg_ptr();
		if(createMessage(curr_json,msg))
		{
			msg->setHandler(getHandler(MessageMap[messageHeader.msg_type]));
			output.push_back(msg);
		}
	}		//for loop

	return true;
}

} /* namespace roadrunner */
} /* namespace sim_mob */
