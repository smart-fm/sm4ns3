//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "message_base.h"

#include <string>
#include <vector>
#include <map>

namespace sm4ns3 {

//Sent from Sim Mobility -> ns3
struct OpaqueSendMessage : public sm4ns3::MessageBase {
	std::string fromId; ///<The Agent sending this message.
	std::vector<std::string> toIds; ///<The Agent(s) we are sending this message to.
	bool broadcast; ///<If true, Sim Mobility will overwrite "toIds" with the nearest Agents (using the Aura Manager).
	std::string data; ///<The actual message data
	OpaqueSendMessage(const MessageBase& base) : MessageBase(base) {}
};

//Routed through ns-3, and then sent from ns-3 to Sim Mobility.
struct OpaqueReceiveMessage : public sm4ns3::MessageBase {
	std::string fromId; ///<The Agent sending this message.
	std::string toId; ///<The Agent we are sending this message to.
	std::string data; ///<The actual message data
	OpaqueReceiveMessage(const MessageBase& base) : MessageBase(base) {}
};

struct MulticastMessage : public sm4ns3::MessageBase {
	unsigned int sendingAgent;
	std::vector<unsigned int> recipients;
	std::string msgData;
	MulticastMessage(const MessageBase& base) : MessageBase(base) {}
};

struct AgentsInfoMessage : public sm4ns3::MessageBase {
	std::vector<std::string> addAgentIds; ///<Agent IDs to add
	std::vector<std::string> remAgentIds; ///<Agent IDs to remove
	AgentsInfoMessage(const MessageBase& base) : MessageBase(base) {}
};

struct AllLocationsMessage : public sm4ns3::MessageBase {
	std::map<std::string, DPoint> agentLocations; ///<Maps agentID=>(x,y) updates for locations.
	AllLocationsMessage(const MessageBase& base) : MessageBase(base) {}
};


}

