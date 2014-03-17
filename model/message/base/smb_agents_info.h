//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
//#include "smb_message_base.h"
#include "smb_message_handler_base.h"

namespace sim_mob {

class MSG_Agents_Info : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	Handler * newHandler();
	MSG_Agents_Info(msg_data_t data_);
	MSG_Agents_Info();
	msg_ptr clone(msg_data_t& data_);
};

class HDL_Agents_Info : public Handler {

public:
	void handle(msg_ptr message_,Broker*);
};

} /* namespace sim_mob */
