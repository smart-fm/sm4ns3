//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_message_factory_base.h"

#include "smb_rr_casts.h"
#include "smb_agents_info.h"
#include "smb_all_location.h"
#include "smb_time_info.h"
#include "smb_readytoreceive_message.h"


sm4ns3::MessageFactory::MessageFactory() 
{
	MessageMap["MULTICAST"] = MULTICAST;
	MessageMap["UNICAST"] = UNICAST;
	MessageMap["ALL_LOCATIONS_DATA"] = ALL_LOCATIONS_DATA;
	MessageMap["READY_TO_RECEIVE"] = READY_TO_RECEIVE;
	MessageMap["AGENTS_INFO"] = AGENTS_INFO;
	MessageMap["TIME_DATA"] = TIME_DATA;

	//Register handlers in advance
	HandlerMap[MULTICAST] = new sm4ns3::HDL_MULTICAST();
	HandlerMap[UNICAST] = new sm4ns3::HDL_UNICAST();
	HandlerMap[ALL_LOCATIONS_DATA] = new sm4ns3::HDL_All_Location();
	HandlerMap[AGENTS_INFO] = new sm4ns3::HDL_Agents_Info();
	HandlerMap[TIME_DATA] = new sm4ns3::NullHandler();
}

sm4ns3::MessageFactory::~MessageFactory() 
{
	//Reclaim handlers
	for (std::map<MessageType, sm4ns3::Handler*>::const_iterator it=HandlerMap.begin(); it!=HandlerMap.end(); it++) {
		delete it->second;
	}
	HandlerMap.clear();
}


const sm4ns3::Handler* sm4ns3::MessageFactory::getHandler(const std::string& msgType)
{
	std::map<std::string, MessageType>::const_iterator typeIt = MessageMap.find(msgType);
	if (typeIt!=MessageMap.end()) {
		std::map<MessageType, sm4ns3::Handler*>::iterator it = HandlerMap.find(typeIt->second);
		if(it!=HandlerMap.end() && it->second) {
			return it->second;
		}
	}

	throw std::runtime_error("Unknown handler for message type.");
}


/*bool sm4ns3::MessageFactory::createSingleMessage(const Json::Value& input, msg_ptr& output)
{
	msg_header messageHeader;
	if (!sm4ns3::JsonParser::parseMessageHeader(input, messageHeader)) {
		std::cout <<"MessageFactory::createMessage=>parseMessageHeader Failed 1\n";
		return false;
	}

	//TODO: Further cleanup required.
	if (messageHeader.msg_type=="MULTICAST") {
		output.reset(new sm4ns3::MSG_MULTICAST(input, messageHeader));
	} else if (messageHeader.msg_type=="UNICAST") {
		output.reset(new sm4ns3::MSG_UNICAST(input, messageHeader));
	} else if (messageHeader.msg_type=="ALL_LOCATIONS_DATA") {
		output.reset(new MSG_All_Location(input, messageHeader));
	} else if (messageHeader.msg_type=="READY_TO_RECEIVE") {
		output.reset(new MSG_READY_TO_RECEIVE(input, messageHeader));
	} else if (messageHeader.msg_type=="AGENTS_INFO") {
		output.reset(new MSG_Agents_Info(input, messageHeader));
	} else if (messageHeader.msg_type=="TIME_DATA") {
		output.reset(new MSG_Time(input, messageHeader));
	} else {
		throw std::runtime_error("Unknown message type; can't create message.");
	}

	return true;
}*/


/*bool sm4ns3::MessageFactory::createMessage(const std::string &input, std::vector<msg_ptr>& output)
{
	Json::Value root;
	sm4ns3::pckt_header packetHeader;
	if(!sm4ns3::JsonParser::parsePacketHeader(input, packetHeader, root)) {
		return false;
	}

	if(!sm4ns3::JsonParser::getPacketMessages(input,root)) {
		return false;
	}

	//Parse each individual message.
	for (unsigned int index = 0; index < root.size(); index++) {
		msg_header messageHeader;
		if (!sm4ns3::JsonParser::parseMessageHeader(root[index], messageHeader)) {
			continue;
		}

		const Json::Value& curr_json = root[index];
		msg_ptr msg = msg_ptr();
		if(createSingleMessage(curr_json,msg)) {
			//msg->setHandler(getHandler(MessageMap[messageHeader.msg_type]));
			output.push_back(msg);
		}
	}

	return true;
}*/


