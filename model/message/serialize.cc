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

	messages_v0.push_back(msg);
}


void sm4ns3::MessageConglomerate::addMessage(int offset, int length, const std::string& msgStr)
{
	if (!NEW_BUNDLES) { throw std::runtime_error("Error, attempting to construct v1 MessageConglomerate."); }
	if (offsets_v1.empty() && msgStr.empty()) { throw std::runtime_error("Error; msgString must be non-empty for the first message."); }
	
	offsets_v1.push_back(std::make_pair(offset, length));
	if (!msgStr.empty()) {
		messages_v1 = msgStr;
	}
}

int sm4ns3::MessageConglomerate::getCount() const
{
	if (NEW_BUNDLES) {
		return offsets_v1.size();
	} else {
		return messages_v0.size();
	}
}

const Json::Value& sm4ns3::MessageConglomerate::getMessage(int msgNumber) const
{
	if (NEW_BUNDLES) { throw std::runtime_error("Error, attempting to retrieve v0 MessageConglomerate."); }

	return messages_v0.at(msgNumber);
}

void sm4ns3::MessageConglomerate::getMessage(int msgNumber, int& offset, int& length) const
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

	//Add message lengths.
	for (std::vector<unsigned int>::const_iterator it=ongoing.vHead.msgLengths.begin(); it!=ongoing.vHead.msgLengths.end(); it++) {
		vHead[v_off] = (unsigned char)(((*it)>>16)&0xFF);
		vHead[v_off+1] = (unsigned char)(((*it)>>8)&0xFF);
		vHead[v_off+2] = (unsigned char)(((*it))&0xFF);
		v_off += 3; //Note to Java developers: do the incrementation after in C++; DON'T do ++ multiple times in one line.
	}

	//Add sender ID, destID
	for (size_t i=0; i<ongoing.vHead.sendId.size(); i++) {
		vHead[v_off++] = ongoing.vHead.sendId[i];
	}
	for (size_t i=0; i<ongoing.vHead.destId.size(); i++) {
		vHead[v_off++] = ongoing.vHead.destId[i];
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
	pktHeader["NOF_MESSAGES"] = size_to_string(ongoing.vHead.msgLengths.size());

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
	root["PACKET_HEADER"] = pktHeader;
	root["DATA"] = dataArr;
	res = Json::FastWriter().write(root);

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

	//A series of 3-byte message field lengths follows.
	for (int msgId=0; msgId<header.messageCount; msgId++) {
		int msgSize = (((unsigned char)msgStr[i])<<16) | (((unsigned char)msgStr[i+1])<<8) | ((unsigned char)msgStr[i+2]);
		i += 3; //Note to Java developers: do the incrementation after in C++; DON'T do ++ multiple times in one line.
		vHead.msgLengths.push_back(msgSize);
	}

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
	if (NEW_BUNDLES) {
		throw std::runtime_error("deserialize_single for NEW_BUNDLES not yet supported."); 
	} else {
		if (!(remConglom.getMessage(0).isMember("msg_type") && remConglom.getMessage(0)["msg_type"] == expectedType)) {
			std::cout <<"Error: unexpected message type (or none).\n";
			return false;
		}
	}

	//Parse it into a base message; we'll deal with the custom properties on our own.
	resMsg = JsonParser::parseMessageBase(remConglom, 0);

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


sm4ns3::MessageBase sm4ns3::JsonParser::parseMessageBase(const MessageConglomerate& msg, int msgNumber)
{
	sm4ns3::MessageBase res;

	if (NEW_BUNDLES) {
		throw std::runtime_error("deserialize_single for NEW_BUNDLES not yet supported."); 
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		//Common properties.
		if (!jsMsg.isMember("msg_type")) {
			throw std::runtime_error("Base message is missing some required parameters."); 
		}

		res.msg_type = jsMsg["msg_type"].asString();
	}
	return res;
}


sm4ns3::AgentsInfoMessage sm4ns3::JsonParser::parseNewAgents(const MessageConglomerate& msg, int msgNumber)
{
	sm4ns3::AgentsInfoMessage res(JsonParser::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported."); 
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		//Add?
		if (jsMsg.isMember("add")) {
			if (!jsMsg["add"].isArray()) { throw std::runtime_error("AgentsInfo add should be an array."); }
			const Json::Value& agents = jsMsg["add"];
			for (unsigned int i=0; i<agents.size(); i++) {
				res.addAgentIds.push_back(agents[i].asUInt());
			}
		}

		//Remove?
		if (jsMsg.isMember("rem")) {
			if (!jsMsg["rem"].isArray()) { throw std::runtime_error("AgentsInfo rem should be an array."); }
			const Json::Value& agents = jsMsg["rem"];
			for (unsigned int i=0; i<agents.size(); i++) {
				res.remAgentIds.push_back(agents[i].asUInt());
			}
		}
	}

	return res;
}



sm4ns3::AllLocationsMessage sm4ns3::JsonParser::parseAllLocations(const MessageConglomerate& msg, int msgNumber)
{
	sm4ns3::AllLocationsMessage res(JsonParser::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported."); 
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		if (!(jsMsg.isMember("locations") && jsMsg["locations"].isArray())) { throw std::runtime_error("Badly formatted AllLocations message [1]."); }

		//Parse each location into our map.
		const Json::Value& locs = jsMsg["locations"];
		for(unsigned int i=0; i<locs.size(); i++) {
			const Json::Value& agInf = locs[i];
			if (!(agInf.isMember("id") && agInf.isMember("x") && agInf.isMember("y"))) { 
				throw std::runtime_error("Badly formatted AllLocations message [2]."); 
			}

			//Retrieve per-agent data, add it.
			unsigned int agId = agInf["id"].asUInt();
			double x = agInf["x"].asDouble();
			double y = agInf["y"].asDouble();
			res.agentLocations[agId] = DPoint(x,y);
		}
	}
	return res;
}


sm4ns3::OpaqueSendMessage sm4ns3::JsonParser::parseOpaqueSend(const MessageConglomerate& msg, int msgNumber)
{
	sm4ns3::OpaqueSendMessage res(JsonParser::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported.");
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

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
	}
	return res;
}


void sm4ns3::JsonParser::makeIdResponse(OngoingSerialization& ongoing, const std::string& token) 
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported."); 
	} else {
		Json::Value res;
		res["msg_type"] = "id_response";
		res["id"] = "987654321";  //TODO: Better unique ID.
		res["token"] = token;
		res["type"] = "ns-3";
		res["services"].append("srv_all_locations");

		//Now append it.
		std::string nextMsg = Json::FastWriter().write(res);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
		ongoing.vHead.sendId = "0"; //TODO: This really shouldn't be 0; that's what Sim Mobility uses.
	}
}

void sm4ns3::JsonParser::makeTickedClient(OngoingSerialization& ongoing)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported."); 
	} else {
		Json::Value res;
		res["msg_type"] = "ticked_client";

		//Now append it.
		std::string nextMsg = Json::FastWriter().write(res);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
		ongoing.vHead.sendId = "0"; //TODO: This really shouldn't be 0; that's what Sim Mobility uses.
	}
}


void sm4ns3::JsonParser::makeAllLocations(OngoingSerialization& ongoing, const std::map<unsigned int, DPoint>& allLocations)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported."); 
	} else {
		Json::Value res;
		res["msg_type"] = "all_locations";

		//Add all "LOCATIONS"
		Json::Value singleAgent;
		for (std::map<unsigned int, DPoint>::const_iterator it=allLocations.begin(); it!=allLocations.end(); it++) {
			singleAgent["id"] = it->first;
			singleAgent["x"] = it->second.x;
			singleAgent["y"] = it->second.y;
			res["locations"].append(singleAgent);
		}

		//Now append it.
		std::string nextMsg = Json::FastWriter().write(res);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
		ongoing.vHead.sendId = "0"; //TODO: This really shouldn't be 0; that's what Sim Mobility uses.
	}
}


void sm4ns3::JsonParser::makeOpaqueSend(OngoingSerialization& ongoing, unsigned int sendAgentId, const std::vector<unsigned int>& receiveAgentIds, const std::string& data)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported."); 
	} else {
		Json::Value res;
		res["msg_type"] = "opaque_send";
		res["from_id"] = "987654321"; //TODO: Better unique ID.
	
		//Add the DATA section
		res["data"] = data;

		//Add all "RECIPIENTS"
		for (std::vector<unsigned int>::const_iterator it=receiveAgentIds.begin(); it!=receiveAgentIds.end(); it++) {
			res["to_ids"].append(*it);
		}

		//Heuristic:
		res["broadcast"] = receiveAgentIds.empty();

		//Now append it.
		std::string nextMsg = Json::FastWriter().write(res);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
		ongoing.vHead.sendId = "0"; //TODO: This really shouldn't be 0; that's what Sim Mobility uses.
	}
}


void sm4ns3::JsonParser::makeGoClient(OngoingSerialization& ongoing, const std::map<unsigned int, WFD_Group>& wfdGroups)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported."); 
	} else {
		//First make the single message.
		Json::Value res;
		res["msg_type"] = "go_client";

		//Multi-group formation.
		for(std::map<unsigned int, WFD_Group>::const_iterator it=wfdGroups.begin(); it!=wfdGroups.end(); it++) {
			Json::Value clientMsg;
			clientMsg["go"] = it->second.GO;
			for(std::vector<unsigned int>::const_iterator it2=it->second.members.begin(); it2!=it->second.members.end(); it2++) {
				clientMsg["clients"].append(*it2);
			}
			res["groups"].append(clientMsg);
		}

		//Now append it.
		std::string nextMsg = Json::FastWriter().write(res);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
		ongoing.vHead.sendId = "0"; //TODO: This really shouldn't be 0; that's what Sim Mobility uses.
	}
}

void sm4ns3::JsonParser::makeUnknownJSON(OngoingSerialization& ongoing, const Json::Value& json)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported."); 
	} else {
		//Just append, and hope it's formatted correctly.
		std::string nextMsg = Json::FastWriter().write(json);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
		ongoing.vHead.sendId = "0"; //TODO: This really shouldn't be 0; that's what Sim Mobility uses.
	}
}





