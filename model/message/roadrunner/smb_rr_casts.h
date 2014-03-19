#pragma once

#include "smb_message_base.h"
#include "smb_message_handler_base.h"

#include <vector>

namespace sim_mob {
namespace roadrunner {

struct UnicastMessage : public sim_mob::MessageBase {
	std::string receiver; ///<Who to send this to.
	UnicastMessage(const MessageBase& base) : MessageBase(base) {}
};

struct MulticastMessage : public sim_mob::MessageBase {
	unsigned int sendingAgent;
	std::vector<unsigned int> recipients;
	std::string msgData;
	MulticastMessage(const MessageBase& base) : MessageBase(base) {}
};


/*class MSG_UNICAST : public sim_mob::comm::Message {
public:
	Handler * newHandler();
	MSG_UNICAST(const Json::Value& data_, const sim_mob::msg_header& header);
};*/

class HDL_UNICAST : public Handler {
public:
	virtual void handle(const Json::Value& msg, Broker*) const;
};

/*class MSG_MULTICAST : public sim_mob::comm::Message {
public:
	Handler * newHandler();
	MSG_MULTICAST(const Json::Value& data_, const sim_mob::msg_header& header);
};*/

class HDL_MULTICAST : public Handler {
public:
	virtual void handle(const Json::Value& msg, Broker*) const;
};


}}

