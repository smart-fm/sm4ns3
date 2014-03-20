#pragma once

#include "smb_message_base.h"
#include "smb_message_handler_base.h"

#include <vector>

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


/*class MSG_UNICAST : public sm4ns3::comm::Message {
public:
	Handler * newHandler();
	MSG_UNICAST(const Json::Value& data_, const sm4ns3::msg_header& header);
};*/

class HDL_UNICAST : public Handler {
public:
	virtual void handle(const Json::Value& msg, Broker*) const;
};

/*class MSG_MULTICAST : public sm4ns3::comm::Message {
public:
	Handler * newHandler();
	MSG_MULTICAST(const Json::Value& data_, const sm4ns3::msg_header& header);
};*/

class HDL_MULTICAST : public Handler {
public:
	virtual void handle(const Json::Value& msg, Broker*) const;
};


}

