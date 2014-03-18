#pragma once
//#include "entities/commsim/communicator/message/base/Message.hpp"
//#include "UNICAST_Handler.hpp"
#include "smb_message_base.h"
#include "smb_message_handler_base.h"

namespace sim_mob {
namespace roadrunner {

class MSG_UNICAST : public sim_mob::comm::Message {
	//...
public:
	Handler * newHandler();
//	MSG_UNICAST();
	MSG_UNICAST(const Json::Value& data_, const sim_mob::msg_header& header);
//	msg_ptr  clone(msg_data_t&);
};

class HDL_UNICAST : public Handler {

public:
//	HDL_UNICAST();
	virtual void handle(msg_ptr message_,Broker*) const;
};

class MSG_MULTICAST : public sim_mob::comm::Message {
	//...
public:
//	MSG_MULTICAST();
	Handler * newHandler();
	MSG_MULTICAST(const Json::Value& data_, const sim_mob::msg_header& header);
//	msg_ptr clone(msg_data_t&);
};

class HDL_MULTICAST : public Handler {

public:
//	HDL_MULTICAST();
	virtual void handle(msg_ptr message_,Broker*) const;
};


}/* namespace roadrunner */
} /* namespace sim_mob */
