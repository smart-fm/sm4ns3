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

class MSG_All_Location : public sim_mob::comm::Message {
	//...
public:
//	Handler * newHandler();
	MSG_All_Location(const Json::Value& data_, const sim_mob::msg_header& header);
//	MSG_All_Location();
	virtual ~MSG_All_Location();
};

class HDL_All_Location : public Handler {

public:
	virtual void handle(msg_ptr message_,Broker*) const;
	virtual ~HDL_All_Location();
};

} /* namespace sim_mob */
