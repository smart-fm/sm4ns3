//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "serialize.h"

#include <sstream>
#include <stdexcept>

namespace {
std::string size_to_string(size_t dat) {
	std::ostringstream ss;
	ss <<dat;
	return ss.str();
}
} //End anon namespace


bool sm4ns3::JsonParser::serialize(const std::vector<Json::Value>& messages, std::string& res)
{
	//Build the header.
	Json::Value header;
	header["NOF_MESSAGES"] = size_to_string(messages.size());

	//Build the data section.
	Json::Value data;
	for (std::vector<Json::Value>::const_iterator it=messages.begin(); it!=messages.end(); it++) {
		data.append(*it);
	}

	//Combine, return.
	Json::Value root;
	root["PACKET_HEADER"] = header;
	root["DATA"] = data;
	res = Json::FastWriter().write(root);
	return true;
}

bool sm4ns3::JsonParser::deserialize(const std::string& msgStr, std::vector<Json::Value>& res)
{
	//Parse the message into a Json object.
	Json::Value root; 
	Json::Reader reader;
	if (!reader.parse(msgStr, root, false)) {
		std::cout <<"ERROR: Parsing Packet Header for Failed\n";
		return false;
	}

	//We don't actually need any information from the packet header, but it must exist.
	if (!root.isMember("PACKET_HEADER")) {
		std::cout <<"Packet header not found in input: \"" << msgStr << "\"\n";
		return false;
	}

	//Retrieve the DATA section, which must be an array.
	if (!root.isMember("DATA") && root["DATA"].isArray()) {
		std::cout <<"A 'DATA' section with correct format was not found in input: \"" <<msgStr <<"\"\n";
		return false;
	}

	//Now extract the messages one by one.
	const Json::Value& data = root["DATA"];
	for (unsigned int i=0; i<data.size(); i++) {
		res.push_back(data[i]);
	}
	return true;
}

bool sm4ns3::JsonParser::deserialize_single(const std::string& msgStr, const std::string& expectedType, sm4ns3::MessageBase& resMsg, Json::Value& remProps)
{
	std::vector<Json::Value> msgs;
	if (!JsonParser::deserialize(msgStr, msgs)) {
		std::cout <<"Error deserializing message.\n";
		return false;
	}

	//There should only be one message.
	if (msgs.size() != 1) {
		std::cout <<"Error: expected a single message (" <<expectedType <<").\n";
		return false;
	}

	//Make sure it's actually of the expected type..
	remProps = msgs[0];
	if (!(remProps.isMember("MESSAGE_TYPE") && remProps["MESSAGE_TYPE"] == expectedType)) {
		std::cout <<"Error: unexpected message type (or none).\n";
		return false;
	}

	//Parse it into a base message; we'll deal with the custom properties on our own.
	resMsg = JsonParser::parseMessageBase(remProps);

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


sm4ns3::MessageBase sm4ns3::JsonParser::parseMessageBase(const Json::Value& msg)
{
	//Common properties.
	if (!(msg.isMember("SENDER") && msg.isMember("SENDER_TYPE") && msg.isMember("MESSAGE_TYPE") && msg.isMember("MESSAGE_CAT"))) {
		throw std::runtime_error("Base message is missing some required parameters."); 
	}

	sm4ns3::MessageBase res;
	res.sender_id = msg["SENDER"].asString();
	res.sender_type = msg["SENDER_TYPE"].asString();
	res.msg_type = msg["MESSAGE_TYPE"].asString();
	res.msg_cat = msg["MESSAGE_CAT"].asString();
	return res;
}


sm4ns3::AgentsInfoMessage sm4ns3::JsonParser::parseAgentsInfo(const Json::Value& msg)
{
	sm4ns3::AgentsInfoMessage res(JsonParser::parseMessageBase(msg));

	//Add?
	if (msg.isMember("ADD")) {
		if (!msg["ADD"].isArray()) { throw std::runtime_error("AgentsInfo ADD should be an array."); }
		const Json::Value& agents = msg["ADD"];
		for (unsigned int i=0; i<agents.size(); i++) {
			res.addAgentIds.push_back(agents[i]["AGENT_ID"].asUInt());
		}
	}

	//Remove?
	if (msg.isMember("REMOVE")) {
		if (!msg["REMOVE"].isArray()) { throw std::runtime_error("AgentsInfo REMOVE should be an array."); }
		const Json::Value& agents = msg["REMOVE"];
		for (unsigned int i=0; i<agents.size(); i++) {
			res.remAgentIds.push_back(agents[i]["AGENT_ID"].asUInt());
		}
	}

	return res;
}



sm4ns3::AllLocationsMessage sm4ns3::JsonParser::parseAllLocations(const Json::Value& msg)
{
	sm4ns3::AllLocationsMessage res(JsonParser::parseMessageBase(msg));
	if (!msg.isMember("LOCATIONS")) { throw std::runtime_error("Badly formatted AllLocations message [1]."); }

	//Parse each location into our map.
	const Json::Value& locs = msg["LOCATIONS"];
	for(unsigned int i=0; i<locs.size(); i++) {
		const Json::Value& agInf = locs[i];
		if (!(agInf.isMember("ID") && agInf.isMember("x") && agInf.isMember("y"))) { 
			throw std::runtime_error("Badly formatted AllLocations message [2]."); 
		}

		//Retrieve per-agent data, add it.
		unsigned int agId = agInf["ID"].asUInt();
		double x = agInf["x"].asDouble();
		double y = agInf["y"].asDouble();
		res.agentLocations[agId] = DPoint(x,y);
	}
	return res;
}


sm4ns3::UnicastMessage sm4ns3::JsonParser::parseUnicast(const Json::Value& msg)
{
	sm4ns3::UnicastMessage res(JsonParser::parseMessageBase(msg));
	if (!msg.isMember("RECEIVER")) { throw std::runtime_error("Badly formatted Unicast message."); }

	//Fairly simple.
	res.receiver = msg["RECEIVER"].asString();
	return res;
}


sm4ns3::MulticastMessage sm4ns3::JsonParser::parseMulticast(const Json::Value& msg)
{
	sm4ns3::MulticastMessage res(JsonParser::parseMessageBase(msg));
	if (!(msg.isMember("SENDING_AGENT") && msg.isMember("RECIPIENTS") && msg.isMember("DATA"))) { 
		throw std::runtime_error("Badly formatted Multicast message."); 
	}

	//Save and return.
	res.sendingAgent = msg["SENDING_AGENT"].asUInt();
	res.msgData = msg["MULTICAST_DATA"].asString();
	const Json::Value& recip = msg["RECIPIENTS"];
	for (unsigned int i=0; i<recip.size(); i++) {
		res.recipients.push_back(recip[i].asUInt());
	}
	return res;
}



void sm4ns3::JsonParser::addDefaultMessageProps(Json::Value& msg, const std::string& msgType)
{
	msg["SENDER"] = "0";
	msg["SENDER_TYPE"] = "NS3_SIMULATOR";
	msg["MESSAGE_TYPE"] = msgType;
	msg["MESSAGE_CAT"] = "SYS";
}


Json::Value sm4ns3::JsonParser::makeWhoAmI(const std::string& token) 
{
	Json::Value res;
	addDefaultMessageProps(res, "WHOAMI");
	res["ID"] = "0";
	res["token"] = token;
	res["TYPE"] = "NS3_SIMULATOR";
	res["REQUIRED_SERVICES"].append("SIMMOB_SRV_TIME");
	res["REQUIRED_SERVICES"].append("SIMMOB_SRV_ALL_LOCATIONS");
	res["REQUIRED_SERVICES"].append("SIMMOB_SRV_UNKNOWN");

	return res;
}

Json::Value sm4ns3::JsonParser::makeClientDone() 
{
	Json::Value res;
	addDefaultMessageProps(res, "CLIENT_MESSAGES_DONE");

	return res;
}


