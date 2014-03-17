/*
 * smb_readytoreceive_message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

#include "smb_message_base.h"
#include "smb_message_handler_base.h"

namespace sim_mob {

class MSG_READY_TO_RECEIVE : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	Handler * newHandler();
	MSG_READY_TO_RECEIVE(msg_data_t& data_);
	MSG_READY_TO_RECEIVE();
	msg_ptr clone(msg_data_t&);
};

} /* namespace sim_mob */
