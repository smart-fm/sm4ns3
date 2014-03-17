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

class MSG_Time : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	Handler * newHandler();
	MSG_Time(msg_data_t data_);
	MSG_Time();
	msg_ptr clone(msg_data_t& data_);
	virtual ~MSG_Time();
};

class HDL_Time : public Handler {

public:
	void handle(msg_ptr message_,Broker*);
	virtual ~HDL_Time();
};

} /* namespace sim_mob */
