//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "ns3/log.h"

#include "smb_message_base.h"

namespace sim_mob {


class MessageFactory {
	enum MessageType {
		READY,
		TIME_DATA,
		ALL_LOCATIONS_DATA,
		CLIENT_MESSAGES_DONE,
		READY_TO_RECEIVE,
		AGENTS_INFO,
		MULTICAST,
		UNICAST,
	};

public:
	MessageFactory();
	~MessageFactory();

	//Turn an input string into a list of Json-formatted messages
	bool createMessage(const std::string&, std::vector<sim_mob::msg_ptr>&);

	//Retrieve a message handler for a given message type.
	const sim_mob::Handler* getHandler(const std::string& msgType);

private:
	std::map<std::string, MessageType> MessageMap;
	std::map<MessageType, sim_mob::Handler*> HandlerMap;

public:
	//Make a single message from its Json representation.
	bool createSingleMessage(const Json::Value&, sim_mob::msg_ptr&);
};

}

