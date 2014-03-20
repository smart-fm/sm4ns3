//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "ns3/log.h"


namespace sm4ns3 {

class Handler;


class MessageFactory {
public:
	MessageFactory();
	~MessageFactory();

	//Retrieve a message handler for a given message type.
	const sm4ns3::Handler* getHandler(const std::string& msgType);

private:
	//Handy lookup for handler types.
	std::map<std::string, const sm4ns3::Handler*> HandlerMap;
};

}

