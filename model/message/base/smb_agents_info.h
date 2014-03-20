//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "smb_message_base.h"
#include "smb_message_handler_base.h"

namespace sm4ns3 {

struct AgentsInfoMessage : public sm4ns3::MessageBase {
	std::vector<unsigned int> addAgentIds; ///<Agent IDs to add
	std::vector<unsigned int> remAgentIds; ///<Agent IDs to remove
	AgentsInfoMessage(const MessageBase& base) : MessageBase(base) {}
};

/*class MSG_Agents_Info : public sm4ns3::comm::Message {
public:
//	Handler * newHandler();
	MSG_Agents_Info(const Json::Value& data_, const sm4ns3::msg_header& header);
//	MSG_Agents_Info();
};*/

class HDL_Agents_Info : public Handler {
public:
	virtual void handle(const Json::Value&, Broker*) const;
};

} /* namespace sm4ns3 */
