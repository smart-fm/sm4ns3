//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "serialize.h"

#include <sstream>
#include <stdexcept>

#include "registration.h"

namespace {
std::string size_to_string(size_t dat) {
	std::ostringstream ss;
	ss <<dat;
	return ss.str();
}
} //End anon namespace


void sm4ns3::MessageConglomerate::addMessage(const Json::Value& msg)
{
	if (NEW_BUNDLES) { throw std::runtime_error("Error, attempting to construct v0 MessageConglomerate."); }

	messages_json.push_back(msg);
	message_bases.push_back(MessageBase());
	ParseJsonMessageBase(messages_json.back(), message_bases.back());
}


void sm4ns3::MessageConglomerate::addMessage(int offset, int length, const std::string& msgStr)
{
	if (!NEW_BUNDLES) { throw std::runtime_error("Error, attempting to construct v1 MessageConglomerate."); }
	if (offsets_v1.empty() && msgStr.empty()) { throw std::runtime_error("Error; msgString must be non-empty for the first message."); }
	
	//Save the message string, ONCE
	if (!msgStr.empty()) {
		if (!messages_v1.empty()) {
			throw std::runtime_error("Can't overwrite the message string once set.");
		}
		messages_v1 = msgStr;
	}

	//Make sure our offset is not out of bounds.
	if (offset<0 || offset+length>static_cast<int>(messages_v1.length())) {
		throw std::runtime_error("Can't add message: total length exceeds length of message string.");
	}

	//Save the offset.
	offsets_v1.push_back(std::make_pair(offset, length));

	//Now try to parse the message type.
	messages_json.push_back(Json::Value());
	message_bases.push_back(MessageBase());

	//Check the first character to determine the type (binary/json).
	const char* raw = messages_v1.c_str();
	if (static_cast<unsigned char>(raw[offset]) == 0xBB) {
		throw std::runtime_error("Base (v1) message binary format not yet supported.");
	} else if (static_cast<unsigned char>(raw[offset]) == '{') {
		Json::Reader reader;
		if (!reader.parse(&raw[offset], &raw[offset+length], messages_json.back(), false)) {
			throw std::runtime_error("Parsing JSON message base failed.");
		}

		ParseJsonMessageBase(messages_json.back(), message_bases.back());
	} else {
		throw std::runtime_error("Unable to determine v1 message format (binary or JSON).");
	}
}


void sm4ns3::MessageConglomerate::ParseJsonMessageBase(const Json::Value& root, MessageBase& res)
{
	//Check the type string.
	if (!root.isMember("msg_type")) {
		throw std::runtime_error("Base message is missing required parameter 'msg_type'.");
	}
	res.msg_type = root["msg_type"].asString();
}


int sm4ns3::MessageConglomerate::getCount() const
{
	if (NEW_BUNDLES) {
		return offsets_v1.size();
	} else {
		return messages_json.size();
	}
}

sm4ns3::MessageBase sm4ns3::MessageConglomerate::getBaseMessage(int msgNumber) const
{
	return message_bases.at(msgNumber);
}

const Json::Value& sm4ns3::MessageConglomerate::getJsonMessage(int msgNumber) const
{
	return messages_json.at(msgNumber);
}

void sm4ns3::MessageConglomerate::getRawMessage(int msgNumber, int& offset, int& length) const
{
	if (!NEW_BUNDLES) { throw std::runtime_error("Error, attempting to retrieve v1 MessageConglomerate [1]."); }

	offset = offsets_v1.at(msgNumber).first;
	length = offsets_v1.at(msgNumber).second;
}

const std::string& sm4ns3::MessageConglomerate::getUnderlyingString() const
{
	if (!NEW_BUNDLES) { throw std::runtime_error("Error, attempting to retrieve v1 MessageConglomerate [2]."); }

	return messages_v1;
}


const std::string& sm4ns3::MessageConglomerate::getSenderId() const
{
	return senderId;
}

void sm4ns3::MessageConglomerate::setSenderId(const std::string& id)
{
	senderId = id;
}


void sm4ns3::JsonParser::serialize_begin(OngoingSerialization& ongoing)
{
	serialize_begin(ongoing, "987654321", "0"); //TODO: Better random ID.
}

void sm4ns3::JsonParser::serialize_begin(OngoingSerialization& ongoing, const std::string& sendId, const std::string& destId)
{
	ongoing.vHead.sendId = sendId;
	ongoing.vHead.destId = destId;
	ongoing.vHead.msgLengths.clear();
}

bool sm4ns3::JsonParser::serialize_end(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res)
{
	return NEW_BUNDLES ? serialize_end_v1(ongoing, hRes, res) : serialize_end_v0(ongoing, hRes, res);
}


bool sm4ns3::JsonParser::serialize_end_v1(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res)
{
	//Precalculate the varying header length.
	const size_t varHeadSize = ongoing.vHead.msgLengths.size()*3 + ongoing.vHead.sendId.size() + ongoing.vHead.destId.size();
	unsigned char vHead[varHeadSize];
	size_t v_off = 0; //Current offset into vHead.

	//Add sender ID, destID
	for (size_t i=0; i<ongoing.vHead.sendId.size(); i++) {
		vHead[v_off++] = ongoing.vHead.sendId[i];
	}
	for (size_t i=0; i<ongoing.vHead.destId.size(); i++) {
		vHead[v_off++] = ongoing.vHead.destId[i];
	}

	//Add message lengths.
	for (std::vector<unsigned int>::const_iterator it=ongoing.vHead.msgLengths.begin(); it!=ongoing.vHead.msgLengths.end(); it++) {
		vHead[v_off] = (unsigned char)(((*it)>>16)&0xFF);
		vHead[v_off+1] = (unsigned char)(((*it)>>8)&0xFF);
		vHead[v_off+2] = (unsigned char)(((*it))&0xFF);
		v_off += 3; //Note to Java developers: do the incrementation after in C++; DON'T do ++ multiple times in one line.
	}

	//Sanity check.
	if (v_off != varHeadSize) { throw std::runtime_error("Varying header size not exact; memory corruption may have occurred."); }

	//Combine.
	res = std::string(reinterpret_cast<char*>(vHead), varHeadSize) + ongoing.messages.str();

	//Reflect changes to the bundle header.
	hRes.sendIdLen = ongoing.vHead.sendId.size();
	hRes.destIdLen = ongoing.vHead.destId.size();
	hRes.messageCount = ongoing.vHead.msgLengths.size();
	hRes.remLen = res.size();

	return true;
}


bool sm4ns3::JsonParser::serialize_end_v0(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res)
{
	//Build the header.
	Json::Value pktHeader;
	pktHeader["send_client"] = ongoing.vHead.sendId;
	pktHeader["dest_client"] = ongoing.vHead.destId;

	//Turn the current data string into a Json array. (Inefficient, but that doesn't matter for v0)
	std::string data = "[" + ongoing.messages.str() + "]";
	Json::Value dataArr; 
	Json::Reader reader;
	if (!(reader.parse(data, dataArr, false) && dataArr.isArray())) {
		std::cout <<"ERROR: data section cannot be represented as array\n";
		return false;
	} 

	//Combine.
	Json::Value root;
	root["header"] = pktHeader;
	root["messages"] = dataArr;
	res = JsonSingleLineWriter(!NEW_BUNDLES).write(root);

	//Reflect changes to the bundle header.
	hRes.sendIdLen = ongoing.vHead.sendId.size();
	hRes.destIdLen = ongoing.vHead.destId.size();
	hRes.messageCount = ongoing.vHead.msgLengths.size();
	hRes.remLen = res.size();

	return true;
}


bool sm4ns3::JsonParser::deserialize(const BundleHeader& header, const std::string& msgStr, MessageConglomerate& res)
{
	return NEW_BUNDLES ? deserialize_v1(header, msgStr, res) : deserialize_v0(msgStr, res);
}


bool sm4ns3::JsonParser::deserialize_v0(const std::string& msgStr, MessageConglomerate& res)
{
	//Parse the message into a Json object.
	Json::Value root; 
	Json::Reader reader;
	if (!reader.parse(msgStr, root, false)) {
		std::cout <<"ERROR: Parsing Packet Header for Failed\n";
		return false;
	}

	//Retrieve the sender ID from the packet header.
	if (!root.isMember("header") && root["header"].isMember("send_client")) {
		std::cout <<"Packet header not found in input: \"" << msgStr << "\"\n";
		return false;
	}
	res.setSenderId(root["header"]["send_client"].asString());

	//Retrieve the messages section, which must be an array.
	if (!root.isMember("messages") && root["messages"].isArray()) {
		std::cout <<"A 'messages' section with correct format was not found in input: \"" <<msgStr <<"\"\n";
		return false;
	}

	//Now extract the messages one by one.
	const Json::Value& data = root["messages"];
	for (unsigned int i=0; i<data.size(); i++) {
		res.addMessage(data[i]);
	}
	return true;
}

bool sm4ns3::JsonParser::deserialize_v1(const BundleHeader& header, const std::string& msgStr, MessageConglomerate& res)
{
	//First, we need to read the variable-length header:
	VaryHeader vHead;
	int i = 0; //Keep track of our first message offset

	//Following this are the sendId and destId strings.
	std::stringstream idStr;
	for (int sz=0; sz<header.sendIdLen; sz++) {
		idStr <<msgStr[i++];
	}
	vHead.sendId = idStr.str();
	res.setSenderId(vHead.sendId);
	idStr.str("");
	for (int sz=0; sz<header.destIdLen; sz++) {
		idStr <<msgStr[i++];
	}
	vHead.destId = idStr.str();

	//A series of 3-byte message field lengths follows.
	for (int msgId=0; msgId<header.messageCount; msgId++) {
		int msgSize = (((unsigned char)msgStr[i])<<16) | (((unsigned char)msgStr[i+1])<<8) | ((unsigned char)msgStr[i+2]);
		i += 3; //Note to Java developers: do the incrementation after in C++; DON'T do ++ multiple times in one line.
		vHead.msgLengths.push_back(msgSize);
	}
	
	//At this point, we have all the information we require. The only thing left to do is update the MessageConglomerate's cache of <offset,length> pairs for each message.
	for (std::vector<unsigned int>::const_iterator it=vHead.msgLengths.begin(); it!=vHead.msgLengths.end(); it++) {
		res.addMessage(i, *it, it==vHead.msgLengths.begin()?msgStr:"");
		i += *it;
	}

	return true;
}


bool sm4ns3::JsonParser::deserialize_single(const BundleHeader& header, const std::string& msgStr, const std::string& expectedType, sm4ns3::MessageBase& resMsg, MessageConglomerate& remConglom)
{
	if (!JsonParser::deserialize(header, msgStr, remConglom)) {
		std::cout <<"Error deserializing message.\n";
		return false;
	}

	//There should only be one message.
	if (remConglom.getCount() != 1) {
		std::cout <<"Error: expected a single message (" <<expectedType <<").\n";
		return false;
	}

	//Make sure it's actually of the expected type.
	if (remConglom.getBaseMessage(0).msg_type!= expectedType) {
		std::cout <<"Error: unexpected message type (or none).\n";
		return false;
	}

	//Parse it into a base message; we'll deal with the custom properties on our own.
	resMsg = remConglom.getBaseMessage(0);

	return true;
}


bool sm4ns3::JsonParser::parseJSON(const std::string& input, Json::Value &output) 
{
	Json::Reader reader;
	bool parsedSuccess = reader.parse(input, output, false);
	if (!parsedSuccess) {
		std::cout <<"parseJSON() failed.\n";
		return false;
	}
	return true;
}


sm4ns3::AgentsInfoMessage sm4ns3::JsonParser::parseNewAgents(const MessageConglomerate& msg, int msgNumber)
{
	sm4ns3::AgentsInfoMessage res(msg.getBaseMessage(msgNumber));

	//We are either parsing this as JSON, or as binary; version number doesn't matter in this case.
	const Json::Value& jsMsg = msg.getJsonMessage(msgNumber);
	if (!jsMsg.isNull()) {
		//Add?
		if (jsMsg.isMember("add")) {
			if (!jsMsg["add"].isArray()) { throw std::runtime_error("AgentsInfo add should be an array."); }
			const Json::Value& agents = jsMsg["add"];
			for (unsigned int i=0; i<agents.size(); i++) {
				res.addAgentIds.push_back(agents[i].asString());
			}
		}

		//Remove?
		if (jsMsg.isMember("rem")) {
			if (!jsMsg["rem"].isArray()) { throw std::runtime_error("AgentsInfo rem should be an array."); }
			const Json::Value& agents = jsMsg["rem"];
			for (unsigned int i=0; i<agents.size(); i++) {
				res.remAgentIds.push_back(agents[i].asString());
			}
		}
	} else {
		throw std::runtime_error("parse() for binary messages not yet supported.");
	}

	return res;
}



sm4ns3::AllLocationsMessage sm4ns3::JsonParser::parseAllLocations(const MessageConglomerate& msg, int msgNumber)
{
	sm4ns3::AllLocationsMessage res(msg.getBaseMessage(msgNumber));

	//We are either parsing this as JSON, or as binary; version number doesn't matter in this case.
	const Json::Value& jsMsg = msg.getJsonMessage(msgNumber);
	if (!jsMsg.isNull()) {
		if (!(jsMsg.isMember("locations") && jsMsg["locations"].isArray())) { throw std::runtime_error("Badly formatted AllLocations message [1]."); }

		//Parse each location into our map.
		const Json::Value& locs = jsMsg["locations"];
		for(unsigned int i=0; i<locs.size(); i++) {
			const Json::Value& agInf = locs[i];
			if (!(agInf.isMember("id") && agInf.isMember("x") && agInf.isMember("y"))) { 
				throw std::runtime_error("Badly formatted AllLocations message [2]."); 
			}

			//Retrieve per-agent data, add it.
			std::string agId = agInf["id"].asString();
			double x = agInf["x"].asDouble();
			double y = agInf["y"].asDouble();
			res.agentLocations[agId] = DPoint(x,y);
		}
	} else {
		throw std::runtime_error("parse() for binary messages not yet supported.");
	}
	return res;
}


sm4ns3::OpaqueSendMessage sm4ns3::JsonParser::parseOpaqueSend(const MessageConglomerate& msg, int msgNumber)
{
	sm4ns3::OpaqueSendMessage res(msg.getBaseMessage(msgNumber));

	//We are either parsing this as JSON, or as binary; version number doesn't matter in this case.
	const Json::Value& jsMsg = msg.getJsonMessage(msgNumber);
	if (!jsMsg.isNull()) {
		if (!(jsMsg.isMember("from_id") && jsMsg.isMember("to_ids") && jsMsg.isMember("broadcast") && jsMsg.isMember("data") && jsMsg["to_ids"].isArray())) {
			throw std::runtime_error("Badly formatted OPAQUE_SEND message.");
		}

		//Fairly simple.
		res.fromId = jsMsg["from_id"].asString();
		res.broadcast = jsMsg["broadcast"].asBool();
		res.data = jsMsg["data"].asString();
		const Json::Value& toIds = jsMsg["to_ids"];
		for (unsigned int i=0; i<toIds.size(); i++) {
			res.toIds.push_back(toIds[i].asString());
		}

		//Fail-safe
		if (res.broadcast && !res.toIds.empty()) {
			throw std::runtime_error("Cannot call opaque_send with both \"broadcast\" as true and a non-empty toIds list.");
		}
	} else {
		throw std::runtime_error("parse() for binary messages not yet supported.");
	}
	return res;
}


void sm4ns3::JsonParser::makeIdResponse(OngoingSerialization& ongoing, const std::string& token) 
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() for binary messages not yet supported."); 
	} else {
		Json::Value res;
		res["msg_type"] = "id_response";
		res["id"] = "987654321";  //TODO: Better unique ID.
		res["token"] = token;
		res["type"] = "ns-3";
		res["services"].append("srv_all_locations");

		//Serialize it.
		std::string nextMsg = JsonSingleLineWriter(!NEW_BUNDLES).write(res);

		//Keep the header up-to-date.
		addGeneric(ongoing, nextMsg);
	}
}

void sm4ns3::JsonParser::makeTickedClient(OngoingSerialization& ongoing)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() for binary messages not yet supported.");
	} else {
		Json::Value res;
		res["msg_type"] = "ticked_client";

		//Serialize it.
		std::string nextMsg = JsonSingleLineWriter(!NEW_BUNDLES).write(res);

		//Keep the header up-to-date.
		addGeneric(ongoing, nextMsg);
	}
}


void sm4ns3::JsonParser::makeAllLocations(OngoingSerialization& ongoing, const std::map<std::string, DPoint>& allLocations)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() for binary messages not yet supported.");
	} else {
		Json::Value res;
		res["msg_type"] = "all_locations";

		//Add all "LOCATIONS"
		Json::Value singleAgent;
		for (std::map<std::string, DPoint>::const_iterator it=allLocations.begin(); it!=allLocations.end(); it++) {
			singleAgent["id"] = it->first;
			singleAgent["x"] = it->second.x;
			singleAgent["y"] = it->second.y;
			res["locations"].append(singleAgent);
		}

		//Serialize it.
		std::string nextMsg = JsonSingleLineWriter(!NEW_BUNDLES).write(res);

		//Keep the header up-to-date.
		addGeneric(ongoing, nextMsg);
	}
}


void sm4ns3::JsonParser::makeOpaqueSend(OngoingSerialization& ongoing, const std::string& sendAgentId, const std::vector<std::string>& receiveAgentIds, const std::string& data)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() for binary messages not yet supported.");
	} else {
		Json::Value res;
		res["msg_type"] = "opaque_send";
		res["from_id"] = "987654321"; //TODO: Better unique ID.
	
		//Add the DATA section
		res["data"] = data;

		//Add all "RECIPIENTS"
		for (std::vector<std::string>::const_iterator it=receiveAgentIds.begin(); it!=receiveAgentIds.end(); it++) {
			res["to_ids"].append(*it);
		}

		//Heuristic:
		res["broadcast"] = receiveAgentIds.empty();

		//Serialize it.
		std::string nextMsg = JsonSingleLineWriter(!NEW_BUNDLES).write(res);

		//Keep the header up-to-date.
		addGeneric(ongoing, nextMsg);
	}
}


void sm4ns3::JsonParser::makeGoClient(OngoingSerialization& ongoing, const std::map<std::string, WFD_Group>& wfdGroups)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() for binary messages not yet supported.");
	} else {
		//First make the single message.
		Json::Value res;
		res["msg_type"] = "go_client";

		//Multi-group formation.
		for(std::map<std::string, WFD_Group>::const_iterator it=wfdGroups.begin(); it!=wfdGroups.end(); it++) {
			Json::Value clientMsg;
			clientMsg["go"] = it->second.GO;
			for(std::vector<std::string>::const_iterator it2=it->second.members.begin(); it2!=it->second.members.end(); it2++) {
				clientMsg["clients"].append(*it2);
			}
			res["groups"].append(clientMsg);
		}

		//Serialize it.
		std::string nextMsg = JsonSingleLineWriter(!NEW_BUNDLES).write(res);

		//Keep the header up-to-date.
		addGeneric(ongoing, nextMsg);
	}
}

void sm4ns3::JsonParser::makeUnknownJSON(OngoingSerialization& ongoing, const Json::Value& json)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() for binary messages not yet supported.");
	} else {
		//Just serialize, and hope it's formatted correctly.
		std::string nextMsg = JsonSingleLineWriter(!NEW_BUNDLES).write(json);

		//Keep the header up-to-date.
		addGeneric(ongoing, nextMsg);
	}
}


void sm4ns3::JsonParser::addGeneric(OngoingSerialization& ongoing, const std::string& msg)
{
	//Just append, and hope it's formatted correctly.
	if (NEW_BUNDLES) {
		ongoing.messages <<msg;
	} else {
		//We actually need to represent a JSON vector.
		ongoing.messages <<(ongoing.messages.str().empty()?"":",") <<msg;
	}

	//Keep the header up-to-date.
	ongoing.vHead.msgLengths.push_back(msg.size());
}



///////////////////////////////////
// JsonSingleLineWriter methods.
///////////////////////////////////


sm4ns3::JsonSingleLineWriter::JsonSingleLineWriter(bool appendNewline) : yamlCompatiblityEnabled_( false ), appendNewline(appendNewline)
{
}


void sm4ns3::JsonSingleLineWriter::enableYAMLCompatibility()
{
   yamlCompatiblityEnabled_ = true;
}


std::string sm4ns3::JsonSingleLineWriter::write(const Json::Value &root)
{
   document_.str("");
   writeValue( root );
   if (appendNewline) {    //NOTE: This is the first major difference between JsonSingleLineWriter and FastWriter.
	   document_ << "\n";
   }
   return document_.str(); //NOTE: This (using a stringstream instead of a string) is the second major difference.
}


void sm4ns3::JsonSingleLineWriter::writeValue(const Json::Value &value)
{
   switch ( value.type() )
   {
   case Json::nullValue:
      document_ << "null";
      break;
   case Json::intValue:
      document_ << Json::valueToString( value.asInt() );
      break;
   case Json::uintValue:
      document_ << Json::valueToString( value.asUInt() );
      break;
   case Json::realValue:
      document_ << Json::valueToString( value.asDouble() );
      break;
   case Json::stringValue:
      document_ << Json::valueToQuotedString( value.asCString() );
      break;
   case Json::booleanValue:
      document_ << Json::valueToString( value.asBool() );
      break;
   case Json::arrayValue:
      {
         document_ << "[";
         int size = value.size();
         for ( int index =0; index < size; ++index )
         {
            if ( index > 0 )
               document_ << ",";
            writeValue( value[index] );
         }
         document_ << "]";
      }
      break;
   case Json::objectValue:
      {
         Json::Value::Members members( value.getMemberNames() );
         document_ << "{";
         for ( Json::Value::Members::iterator it = members.begin();
               it != members.end();
               ++it )
         {
            const std::string &name = *it;
            if ( it != members.begin() )
               document_ << ",";
            document_ << Json::valueToQuotedString( name.c_str() );
            document_ << (yamlCompatiblityEnabled_ ? ": " : ":");
            writeValue( value[name] );
         }
         document_ << "}";
      }
      break;
   }
}


