#pragma once
//#include "entities/commsim/communicator/message/base/Message.hpp"
//#include "UNICAST_Handler.hpp"
#include "smb_message_base.h"
#include "smb_message_handler_base.h"

namespace sim_mob {
namespace roadrunner {

class MSG_UNICAST : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	Handler * newHandler();
	MSG_UNICAST();
	MSG_UNICAST(msg_data_t& data_);
	msg_ptr  clone(msg_data_t&);
};

class HDL_UNICAST : public Handler {

public:
//	HDL_UNICAST();
	void handle(msg_ptr message_,Broker*);
};

class MSG_MULTICAST : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	MSG_MULTICAST();
	Handler * newHandler();
	MSG_MULTICAST(msg_data_t data_);
	msg_ptr clone(msg_data_t&);
};

class HDL_MULTICAST : public Handler {

public:
//	HDL_MULTICAST();
	void handle(msg_ptr message_,Broker*);
};


}/* namespace roadrunner */
} /* namespace sim_mob */
