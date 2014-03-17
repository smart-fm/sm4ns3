/*
 * SYS_MSG_Factory.cpp
 *
 *  Created on: Jul 26, 2013
 *      Author: vahid
 */

#include "smb_system_factory.h"
#include "smb_agents_info.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include<jsoncpp/json/json.h>
#include <stdexcept>

namespace sim_mob {

SYS_MSG_Factory::SYS_MSG_Factory() {
	// TODO Auto-generated constructor stub
	MessageMap = boost::assign::map_list_of
//			("READY" , READY)
			("ALL_LOCATIONS_DATA", ALL_LOCATIONS_DATA)
			("READY_TO_RECEIVE",READY_TO_RECEIVE)
			("TIME_DATA", TIME_DATA)
			("AGENTS_INFO", AGENTS_INFO);

	registerMessage(std::string("ALL_LOCATIONS_DATA"), msg_ptr (new MSG_All_Location()));
	registerMessage(std::string("READY_TO_RECEIVE"), msg_ptr (new MSG_READY_TO_RECEIVE()));
	registerMessage(std::string("AGENTS_INFO"), msg_ptr (new MSG_Agents_Info()));
	registerMessage(std::string("TIME_DATA"), msg_ptr (new MSG_Time()));
}

SYS_MSG_Factory::~SYS_MSG_Factory() {}
//gets a handler either from a chche or by creating a new one
hdlr_ptr  SYS_MSG_Factory::getHandler(MessageType type){
	//debug:
	std::pair<std::string,MessageType > p;
	std::string type_str = "";
	BOOST_FOREACH(p, MessageMap)
	{
		if(p.second == type)
		{
			type_str = p.first;
			break;
		}
	}

	///////////////
	hdlr_ptr handler;
	//if handler is already registered && the registered handler is not null
	std::map<MessageType, hdlr_ptr >::iterator it = HandlerMap.find(type);
	if((it != HandlerMap.end())&&((*it).second!= 0))
	{
		//get the handler ...
		handler = (*it).second;
//		NS_LOG_UNCOND( "SYS_MSG_Factory::getHandler=> handler found for_ " << type_str );
	}
	else
	{
		//else, create a cache entry ...
		bool typeFound = true;
		switch(type)
		{
		case ALL_LOCATIONS_DATA:
			handler.reset(new sim_mob::HDL_All_Location());
			break;
		case AGENTS_INFO:
			handler.reset(new sim_mob::HDL_Agents_Info());
			break;
		case TIME_DATA:
			handler.reset(new sim_mob::HDL_Time());
			break;
		default:
			typeFound = false;
			NS_LOG_UNCOND( "SYS_MSG_Factory::getHandler=>no handler for " << type_str );
		}
		//register this baby
		if(typeFound)
		{
//			NS_LOG_UNCOND( "SYS_MSG_Factory::getHandler=> handler found for " << type_str );
			HandlerMap[type] = handler;
		}
	}


	return handler;
}

bool SYS_MSG_Factory::createMessage(msg_data_t &input, msg_ptr& output)
{
	msg_header messageHeader;
	if (!sim_mob::JsonParser::parseMessageHeader(input, messageHeader)) {
		NS_LOG_UNCOND( "createMessage(SYS) Failed" );
		return false;
	}
	NS_LOG_UNCOND("createMessage(SYS) for [" << messageHeader.msg_type << "]");
	bool res;
	const msg_ptr prototype = getPrototype(messageHeader.msg_type, res);

	if(!res)
	{
		NS_LOG_UNCOND( "createMessage(SYS) Failed-" );
		return false;
	}
	output = prototype->clone(input);
	output->setHandler(getHandler(MessageMap[messageHeader.msg_type]));
	return true;
}
//creates a message with correct format + assigns correct handler
//todo improve the function to handle array of messages stored in the input string
 bool SYS_MSG_Factory::createMessage(std::string &input, std::vector<msg_ptr>& output)
{
	 NS_LOG_UNCOND("SYS_MSG_Factory::createMessage: '" << input << "'");
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
		NS_LOG_UNCOND( "CreateMessage::" << index << ": (" <<  messageHeader.msg_type << ")" );
		msg_ptr msg = msg_ptr();
		if(createMessage(curr_json,msg))
		{
			msg->setHandler(getHandler(MessageMap[messageHeader.msg_type]));
			output.push_back(msg);
		}
		else
		{
			NS_LOG_UNCOND( "SYS_MSG_Factory::CreateMessage=> for [" <<  messageHeader.msg_type << " failed" );
		}
	}		//for loop

	return true;
}

} /* namespace sim_mob */
