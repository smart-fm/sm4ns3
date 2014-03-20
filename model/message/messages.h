//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "message_base.h"

#include <string>
#include <vector>
#include <map>

namespace sm4ns3 {

struct UnicastMessage : public sm4ns3::MessageBase {
	std::string receiver; ///<Who to send this to.
	UnicastMessage(const MessageBase& base) : MessageBase(base) {}
};

struct MulticastMessage : public sm4ns3::MessageBase {
	unsigned int sendingAgent;
	std::vector<unsigned int> recipients;
	std::string msgData;
	MulticastMessage(const MessageBase& base) : MessageBase(base) {}
};

struct AgentsInfoMessage : public sm4ns3::MessageBase {
	std::vector<unsigned int> addAgentIds; ///<Agent IDs to add
	std::vector<unsigned int> remAgentIds; ///<Agent IDs to remove
	AgentsInfoMessage(const MessageBase& base) : MessageBase(base) {}
};

struct AllLocationsMessage : public sm4ns3::MessageBase {
	std::map<unsigned int, DPoint> agentLocations; ///<Maps agentID=>(x,y) updates for locations.
	AllLocationsMessage(const MessageBase& base) : MessageBase(base) {}
};


}

