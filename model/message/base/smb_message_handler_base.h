//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "smb_message_base.h"

namespace sim_mob {

class Broker;

class Handler {
public:
	virtual ~Handler() {}
	virtual void handle(msg_ptr message_,Broker*) const = 0;
};

}

