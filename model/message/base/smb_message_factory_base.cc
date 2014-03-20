//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_message_factory_base.h"

#include "handlers.h"


sm4ns3::MessageFactory::MessageFactory() 
{
	//Register all known handlers.
	HandlerMap["MULTICAST"] = new sm4ns3::MulticastHandler();
	HandlerMap["UNICAST"] = new sm4ns3::UnicastHandler();
	HandlerMap["ALL_LOCATIONS_DATA"] = new sm4ns3::AllLocationHandler();
	HandlerMap["AGENTS_INFO"] = new sm4ns3::AgentsInfoHandler();
	HandlerMap["TIME_DATA"] = new sm4ns3::NullHandler();
}

sm4ns3::MessageFactory::~MessageFactory() 
{
	//Reclaim handlers
	for (std::map<std::string, const sm4ns3::Handler*>::const_iterator it=HandlerMap.begin(); it!=HandlerMap.end(); it++) {
		delete it->second;
	}
	HandlerMap.clear();
}


const sm4ns3::Handler* sm4ns3::MessageFactory::getHandler(const std::string& msgType)
{
	std::map<std::string, const sm4ns3::Handler*>::const_iterator it = HandlerMap.find(msgType);
	if (it!=HandlerMap.end()) {
		return it->second;
	}

	throw std::runtime_error("Unknown handler for message type.");
}




