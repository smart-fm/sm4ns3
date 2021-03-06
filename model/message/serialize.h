//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>
#include <string>
#include <sstream>
#include <jsoncpp/json/json.h>

#include "ns3/log.h"
#include "message_base.h"
#include "messages.h"
#include "bundle_version.h"

namespace sm4ns3 {

class JsonParser;
class WFD_Group;


/**
 * A variant of Json::FastWriter that does not append a trailing newline.
 * NOTE: For the purpose of copyright, this class should be considered a
 *       derivative work of Json::FastWriter, (c) 2007-2010 Baptiste Lepilleur,
 *       and included under the terms of the MIT License (same as the JsonCpp project).
 * NOTE: This class is also faster than the FastWriter, since it uses a stringstream to append objects
 *       rather than simple strings.
 */
class JsonSingleLineWriter : public Json::Writer
{
public:
	JsonSingleLineWriter(bool appendNewline);
	virtual ~JsonSingleLineWriter(){}
	void enableYAMLCompatibility();

public: // overridden from Writer
	virtual std::string write( const Json::Value &root);

private:
	void writeValue( const Json::Value &value );

	std::stringstream document_;
	bool yamlCompatiblityEnabled_;
	bool appendNewline;
};


/**
 * A class which represents a full bundle of messages, in v0 or v1 format. 
 * For v0, this is just a vector of Json::Value objects.
 * For v1, this is a string and a vector of <offset,length> pairs describing strings that may be 
 *  Json-formatted objects, or may be binary-formatted. 
 * This distinction exists because v0 messages must be parsed to JSON before they can be understood, 
 *  but v1 objects cannot be parsed to JSON (if they are binary).
 * Functions to retrieve the current v0 or v1 object will throw exceptions if the wrong format is set.
 */
class MessageConglomerate {
public:
	///Used to build; v0
	///Make sure you do not call this function after saving any retrieved references
	void addMessage(const Json::Value& msg);

	///Used to build; v1
	///"msgStr" must not be empty for the very first message.
	///Make sure you do not call this function after saving any retrieved references
	void addMessage(int offset, int length, const std::string& msgStr="");

	///Get message count (both versions)
	int getCount() const;

	///Retrieves the MessageBase for a given message number. Works for v0 and v1. Used for easily extracting the type.
	MessageBase getBaseMessage(int msgNumber) const;

	///Retrieves a Json::Value representing this message. For v0, this is always valid.
	///For v1, this is null if a binary mesage.
	const Json::Value& getJsonMessage(int msgNumber) const;

	///Used to retrieve; v1. Fails on v0. (Use this+underlying string to parse binary messages).
	void getRawMessage(int msgNumber, int& offset, int& length) const;

	///Retriee the underlying message string; v1
	const std::string& getUnderlyingString() const;

	///Set the sender's ID
	void setSenderId(const std::string& id);

	///Retrieve the sender's ID.
	const std::string& getSenderId() const;

private:
	//Helper: deserialize common properties associated with all messages.
	static void ParseJsonMessageBase(const Json::Value& root, MessageBase& res);

	//We include a copy of the sender's ID (destination will always be 0, since MessageConglomerates are only used for received messages.
	std::string senderId;

	//v0 only requires this. v1 will save a "null" Json value for every binary messgae and a Json value for every non-binary.
	std::vector<Json::Value> messages_json;

	//v1 requires a bit more.
	std::string messages_v1;
	std::vector< std::pair<int, int> > offsets_v1; //<start, length>

	//Both have access to this.
	std::vector<MessageBase> message_bases;
};


/**
 * Used to serialize messages one at a time efficiently.
 * You will never have to deal with the internals of this class; basically, just do the following:
 * OngoingSerialization s;
 * serialize_begin(s);
 * makeX(params, s);
 * string res; BundleHeader hRes;
 * serialize_end(s, hRes, res);
 */
class OngoingSerialization {
private:
	VaryHeader vHead;
	std::stringstream messages;  //For v1, it's just the messages one after another. For v0, it's, e.g., "{m1},{m2},{m3}".

	friend class JsonParser;
};



/**
 * This class contains methods that serialize from our Message subclasses to Json and back. It also contains methods for 
 *   serializing a series of Json messages to what is currently called a "packet" (a series of messages with a header).
 * The former set of functions are named as "parseX()" and "makeX()".
 * The latter set of functions are named "serialize()" and "deserialize()", with variants for when a single message is expeted.
 */
class JsonParser {
public:
	///Begin serialization of a series of messages. Call this once, followed by several calls to makeX(), followed by serialize_end().
	///TODO: We can improve efficiency by taking in the total message count, senderID, and destID, and partially building the varying header here. 
	///      We would need to add dummy characters for the message lengths, and then overwrite them later during serialize_end().
	static void serialize_begin(OngoingSerialization& ongoing);

	///Special case of serialize_begin where you need to mutate the send/dest IDs.
	static void serialize_begin(OngoingSerialization& ongoing, const std::string& sendId, const std::string& destId);

	///Finish serialization of a series of messages. See serialize_begin() for usage.
	static bool serialize_end(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res);

	//Deserialize a string containing a PACKET_HEADER and a DATA section into a vecot of JSON objects 
	// representing the data section only. The PACKET_HEADER is dealt with internally.
	static bool deserialize(const BundleHeader& header, const std::string& msgStr, MessageConglomerate& res);

	//Deserialize when only a single message is expected (the "expectedType"). 
	//Returns the MessageBase (since that's all we know) and the full message string (as a Conglomerate with a single item) for additional item retrieval.
	static bool deserialize_single(const BundleHeader& header, const std::string& msgStr, const std::string& expectedType, MessageBase& resMsg, MessageConglomerate& remConglom);

	//Turn a string into a Json::Value object. NOTE: This is only used in one very specific case.
	static bool parseJSON(const std::string& input, Json::Value &output);

	//Deserialize a "new_agents" message. 
	static AgentsInfoMessage parseNewAgents(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an "all_locations" message.
	static AllLocationsMessage parseAllLocations(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an "opaque_send" message.
	static OpaqueSendMessage parseOpaqueSend(const MessageConglomerate& msg, int msgNumber);

	//Serialize an "id_response" message.
	static void makeIdResponse(OngoingSerialization& ongoing, const std::string& token);

	//Serialize a "ticked_client" message.
	static void makeTickedClient(OngoingSerialization& ongoing);

	//Serialize an "all_locations" message (used in the trace runner).
	static void makeAllLocations(OngoingSerialization& ongoing, const std::map<std::string, DPoint>& allLocations);

	//Serialize an "opaque_send" message (used in the trace runner).
	//(The actual client simply mutates the incoming OPAQUE_SEND message, so this function is only used in trace.)
	static void makeOpaqueSend(OngoingSerialization& ongoing, const std::string& sendAgentId, const std::vector<std::string>& receiveAgentIds, const std::string& data, const std::string& format, const std::string& tech);

	//Serialize a "go_client" message.
	static void makeGoClient(OngoingSerialization& ongoing, const std::map<std::string, WFD_Group>& wfdGroups);

	//Serialize an unknown JSON-encoded message.
	//TODO: This function will be removed soon, it is the only incompatibility currently left between v0 and v1.
	static void makeUnknownJSON(OngoingSerialization& ongoing, const Json::Value& json);
private:
	///Helper: deserialize v0
	static bool deserialize_v0(const std::string& msgStr, MessageConglomerate& res);

	///Helper: deserialize v1
	static bool deserialize_v1(const BundleHeader& header, const std::string& msgStr, MessageConglomerate& res);

	///Helper: serialize v0
	static bool serialize_end_v0(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res);

	///Helper: serialize v1
	static bool serialize_end_v1(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res);

	//Helper: keep our header up-to-date
	static void addGeneric(OngoingSerialization& ongoing, const std::string& msg);
};

}

