//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>
#include <jsoncpp/json/json.h>

#include "ns3/log.h"
#include "message_base.h"
#include "messages.h"
#include "bundle_version.h"

namespace sm4ns3 {


/**
 * This class contains methods that serialize from our Message subclasses to Json and back. It also contains methods for 
 *   serializing a series of Json messages to what is currently called a "packet" (a series of messages with a header).
 * The former set of functions are named as "parseX()" and "makeX()".
 * The latter set of functions are named "serialize()" and "deserialize()", with variants for when a single message is expeted.
 */
class JsonParser {
public:
	//Serialize a vector of messages (representing the DATA section of an output set) into
	//a string representing these messages combined with a packet header.
	//This function ensures that the BundleHeader passed in remains up-to-date.
	static bool serialize(const std::vector<Json::Value>& messages, BundleHeader& header, std::string& res);

	//Deserialize a string containing a PACKET_HEADER and a DATA section into a vecot of JSON objects 
	// representing the data section only. The PACKET_HEADER is dealt with internally.
	static bool deserialize(const std::string& msgStr, std::vector<Json::Value>& res);

	//Deserialize when only a single message is expected (the "expectedType"). 
	//Returns the MessageBase (since that's all we know) and a Json::Value with any remaining properties.
	static bool deserialize_single(const std::string& msgStr, const std::string& expectedType, MessageBase& resMsg, Json::Value& remProps);

	//Very basic: just parse a std::string to a JsonValue;
	static bool parseJSON(const std::string& input, Json::Value &output);

	//Deserialize common properties associated with all messages.
	static MessageBase parseMessageBase(const Json::Value& msg);

	//Deserialize an AGENTS_INFO message. 
	static AgentsInfoMessage parseAgentsInfo(const Json::Value& msg);

	//Deserialize an ALL_LOCATIONS message.
	static AllLocationsMessage parseAllLocations(const Json::Value& msg);

	//Deserialize a UNICAST message.
	static UnicastMessage parseUnicast(const Json::Value& msg);

	//Deserialize a MULTICAST message.
	static MulticastMessage parseMulticast(const Json::Value& msg);

	//Add default MessageBase properties to an existing Json-encoded message.
	//(Normally not called externally).
	static void addDefaultMessageProps(Json::Value& msg, const std::string& msgType);

	//Serialize a WHOAMI message.
	static Json::Value makeWhoAmI(const std::string& token);

	//Serialize a CLIENT_MESSAGES_DONE message.
	static Json::Value makeClientDone();

	//Serialize an AGENTS_INFO message (used in the trace runner).
	static Json::Value makeAgentsInfo(const std::vector<unsigned int>& addAgents, const std::vector<unsigned int>& remAgents);

	//Serialize an ALL_LOCATIONS message (used in the trace runner).
	static Json::Value makeAllLocations(const std::map<unsigned int, DPoint>& allLocations);

	//Serialize a MULTICAST message (used in the trace runner).
	//(The actual client simply mutates the incoming MULTICAST message, so this function is only used in trace.)
	static Json::Value makeMulticast(unsigned int sendAgentId, const std::vector<unsigned int>& receiveAgentIds, const std::string& data);
};

}

