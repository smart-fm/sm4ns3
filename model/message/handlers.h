//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <stdexcept>
#include <jsoncpp/json/json.h>

#include "handler_base.h"

namespace sm4ns3 {

///Handy lookup class for handlers.
class HandlerLookup {
public:
	HandlerLookup();
	~HandlerLookup();

	//Retrieve a message handler for a given message type.
	const sm4ns3::Handler* getHandler(const std::string& msgType);

private:
	//Handy lookup for handler types.
	std::map<std::string, const sm4ns3::Handler*> HandlerMap;
};


///A handler that does nothing.
class NullHandler : public Handler {
	virtual ~NullHandler() {}
	virtual void handle(const MessageConglomerate& messages, int msgNumber, Broker* broker) const {}
};

///A handler for a deprecated or un-allowed type.
class InvalidHandler : public Handler {
	virtual ~InvalidHandler() {}
	virtual void handle(const MessageConglomerate& messages, int msgNumber, Broker* broker) const {
		throw std::runtime_error("Attempting to use an invalid message (probably MULTICAST or UNICAST, which were replaced with OPAQUE_SEND and OPAQUE_RECEIVE).");
	}
};

class AllLocationHandler : public Handler {
public:
	virtual void handle(const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class AgentsInfoHandler : public Handler {
public:
	virtual void handle(const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class OpaqueSendHandler : public Handler {
public:
	virtual void handle(const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};



}

