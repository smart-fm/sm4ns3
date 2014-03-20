//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "smb_message_base.h"
#include <jsoncpp/json/json.h>

namespace sm4ns3 {

class Broker;

class Handler {
public:
	virtual ~Handler() {}
	virtual void handle(const Json::Value&, Broker*) const = 0;
};

///A handler that does nothing.
class NullHandler : public Handler {
	virtual ~NullHandler() {}
	virtual void handle(const Json::Value&, Broker*) const {}
};

}

