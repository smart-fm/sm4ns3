/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once
#include "smb_message_base.h"
#include "smb_message_handler_base.h"

namespace sim_mob {

class MSG_All_Location : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	Handler * newHandler();
	MSG_All_Location(msg_data_t data_);
	MSG_All_Location();
	msg_ptr clone(msg_data_t& data_);
	virtual ~MSG_All_Location();
};

class HDL_All_Location : public Handler {

public:
	void handle(msg_ptr message_,Broker*);
	virtual ~HDL_All_Location();
};

} /* namespace sim_mob */
