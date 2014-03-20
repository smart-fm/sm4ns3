//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <jsoncpp/json/json.h>

namespace sm4ns3 {

class Broker;

///A base class for anything that can "handle" a message. Note that the message is passed in 
/// as a Json object, so handlers should proceed to parse the type of message they expect in the 
/// handle() method. (The serialize.h functions can help with this.)
class Handler {
public:
	virtual ~Handler() {}
	virtual void handle(const Json::Value&, Broker*) const = 0;
};

}

