//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <jsoncpp/json/json.h>

#include "handler_base.h"

namespace sm4ns3 {

///A handler that does nothing.
class NullHandler : public Handler {
	virtual ~NullHandler() {}
	virtual void handle(const Json::Value&, Broker*) const {}
};

class AllLocationHandler : public Handler {
public:
	virtual void handle(const Json::Value& msg, Broker* broker) const;
};

class AgentsInfoHandler : public Handler {
public:
	virtual void handle(const Json::Value& msg, Broker* broker) const;
};

class UnicastHandler : public Handler {
public:
	virtual void handle(const Json::Value& msg, Broker* broker) const;
};

class MulticastHandler : public Handler {
public:
	virtual void handle(const Json::Value& msg, Broker* broker) const;
};


}

