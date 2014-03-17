/*
 * HandlerFactory.hpp
 *
 *  Created on: May 7, 2013
 *      Author: vahid
 */

#pragma once
#include "smb_message_base.h"
namespace sim_mob
{
class Broker;
//namespace comm
//{
////Forward Declaration
//class Message;
//}
class Handler
{
public:
	virtual void handle(msg_ptr message_,Broker*) = 0;
};
}//namespace
