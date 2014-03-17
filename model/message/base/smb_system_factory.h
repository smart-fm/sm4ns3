/*
 * SYS_MSG_Factory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#pragma once

#include "smb_message_factory_base.h"
#include "smb_serializer.h"
#include "smb_message_base.h"

#include "smb_all_location.h"
#include "smb_readytoreceive_message.h"
#include "smb_time_info.h"


#include <map>

namespace sim_mob {

class SYS_MSG_Factory : public MessageFactory<std::string&, msg_ptr, msg_data_t>/*MessageFactory<output, input>y*/{
	enum MessageType
	{
		READY = 1,
		TIME_DATA = 3,
		ALL_LOCATIONS_DATA = 4,
		CLIENT_MESSAGES_DONE = 5,
		READY_TO_RECEIVE = 6,
		AGENTS_INFO = 7
	};
	std::map<std::string, SYS_MSG_Factory::MessageType> MessageMap;
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, hdlr_ptr > HandlerMap;
//
//	//prototype for message and their handlers
//	std::map<MessageType, const sim_mob::commRole*> prototypes;
//	std::map<MessageType, const sim_mob::Role*> prototypes;

public:
	SYS_MSG_Factory();
	virtual ~SYS_MSG_Factory();
	bool createMessage(msg_data_t &input, msg_ptr& output);//not used
	bool createMessage(std::string &str, std::vector<msg_ptr>&output);
	hdlr_ptr  getHandler(MessageType);
};

} /* namespace sim_mob */
