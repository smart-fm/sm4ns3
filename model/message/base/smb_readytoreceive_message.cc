/*
 * smb_readytoreceive_message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "smb_readytoreceive_message.h"
namespace sim_mob {
class Handler;

class HDL_CLIENTDONE;
MSG_READY_TO_RECEIVE::MSG_READY_TO_RECEIVE(msg_data_t& data_): Message(data_)
{

}
MSG_READY_TO_RECEIVE::MSG_READY_TO_RECEIVE()
{

}

msg_ptr MSG_READY_TO_RECEIVE::clone(msg_data_t& data_) {
	return msg_ptr (new MSG_READY_TO_RECEIVE(data_));
}

Handler * MSG_READY_TO_RECEIVE::newHandler()
{
	return 0;
}
} /* namespace sim_mob */



