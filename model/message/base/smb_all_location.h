/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

#include "smb_message_base.h"
#include "smb_message_handler_base.h"

#include <map>

namespace sm4ns3 {

//TODO: This might need to go somewhere else.
struct DPoint {
	double x;
	double y;
	DPoint(double x=0.0, double y=0.0) : x(x),y(y) {}
};


struct AllLocationsMessage : public sm4ns3::MessageBase {
	std::map<unsigned int, DPoint> agentLocations; ///<Maps agentID=>(x,y) updates for locations.
	AllLocationsMessage(const MessageBase& base) : MessageBase(base) {}
};



/*class MSG_All_Location : public sm4ns3::comm::Message {
	//...
public:
//	Handler * newHandler();
	MSG_All_Location(const Json::Value& data_, const sm4ns3::msg_header& header);
//	MSG_All_Location();
	virtual ~MSG_All_Location();
};*/

class HDL_All_Location : public Handler {

public:
	virtual void handle(const Json::Value& msg, Broker*) const;
	virtual ~HDL_All_Location();
};

} /* namespace sm4ns3 */
