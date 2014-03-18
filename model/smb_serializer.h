//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <sstream>
#include <set>
#include <jsoncpp/json/json.h>
#include "ns3/log.h"

namespace sim_mob {
struct msg_header {
	std::string sender_id;
	std::string sender_type;
	std::string msg_type;
	std::string msg_cat;

	msg_header() {}
	msg_header(std::string sender_id_, std::string sender_type_, std::string msg_type_, std::string msg_cat_ = "UNK") :
			sender_id(sender_id_), sender_type(sender_type_), msg_type(msg_type_), msg_cat(msg_cat_) {}

};

struct pckt_header {
	//data
	std::string sender_id, sender_type, nof_msgs, size_bytes;
	//constructor(s)
	pckt_header():nof_msgs(std::string("0")) {
	}
	pckt_header(std::string nof_msgs_) :
			nof_msgs(nof_msgs_) {
	}

	pckt_header(int nof_msgs_) {
		std::ostringstream out;
		out << nof_msgs_;
		nof_msgs = out.str();
	}
};

class JsonParser {
public:

	static bool parsePacketHeader(const std::string& input, pckt_header &output, Json::Value &root) 
	{
		Json::Value packet_header;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if (not parsedSuccess) {
			NS_LOG_UNCOND( "Parsing Packet Header for Failed");
			return false;
		}
		int i = 0;
		if (root.isMember("PACKET_HEADER")) {
			packet_header = root["PACKET_HEADER"];
		} else {

			NS_LOG_UNCOND( "Packet header not found.Parsing '" << input << "' Failed" );
			return false;
		}
		i += packet_header.isMember("NOF_MESSAGES") ? 8 : 0;
		if (!((packet_header.isMember("NOF_MESSAGES")))) {
			NS_LOG_UNCOND( "Packet header incomplete[" << i << "].Parsing '" << input << "' Failed" );
			return false;
		}
		output.nof_msgs = packet_header["NOF_MESSAGES"].asString();
		return true;
	}

	static bool parseMessageHeader(const Json::Value& root, msg_header &output) {
		if (!((root.isMember("SENDER")) && (root.isMember("SENDER_TYPE")) && (root.isMember("MESSAGE_TYPE")))) {
			NS_LOG_UNCOND( "Message Header incomplete. Parsing  Failed: "  <<root.toStyledString());

			return false;
		}
		output.sender_id = root["SENDER"].asString();
		output.sender_type = root["SENDER_TYPE"].asString();
		output.msg_type = root["MESSAGE_TYPE"].asString();
		if(root.isMember("MESSAGE_CAT")) {
			output.msg_cat = root["MESSAGE_CAT"].asString();
		}
		return true;
	}

	static bool parseMessageHeader(const std::string& input, msg_header &output) 
	{
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if (not parsedSuccess) {
			NS_LOG_UNCOND( "Parsing '" << input << "' Failed" );
			return false;
		}
		return parseMessageHeader(root, output);
	}

	static bool parseMessage(const std::string& input, Json::Value &output) 
	{
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, output, false);
		if (not parsedSuccess) {
			NS_LOG_UNCOND( "Parsing '" << input << "' Failed" );
			return false;
		}
		return true;
	}

	static bool getPacketMessages(const std::string& input, Json::Value &output) 
	{
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if (not parsedSuccess) {
			NS_LOG_UNCOND( "Parsing '" << input << "' Failed" );
			return false;
		}
		if (!((root.isMember("DATA")) && (root["DATA"].isArray()))) {
			NS_LOG_UNCOND("A 'DATA' section with correct format was not found in the message. Parsing '"
					<< input << "' Failed" );
			return false;
		}
		//actual job
		output = root["DATA"];
		return true;
	}

	static Json::Value createPacketHeader(pckt_header pHeader_ = pckt_header()) {
		Json::Value header;
		header["NOF_MESSAGES"] = pHeader_.nof_msgs;
		return header;
	}
	static Json::Value createMessageHeader(msg_header mHeader_) {
		Json::Value header;
		header["SENDER"] = mHeader_.sender_id;
		header["SENDER_TYPE"] = mHeader_.sender_type;
		header["MESSAGE_TYPE"] = mHeader_.msg_type;
		header["MESSAGE_CAT"] = mHeader_.msg_cat;
		return header;
	}
	static std::string makeWhoAreYouPacket() {
		Json::Value whoAreYou_Packet_Header = createPacketHeader(
				pckt_header("1"));
		Json::Value whoAreYou = createMessageHeader(
				msg_header("0", "SIMMOBILITY", "WHOAREYOU", "SYS"));
		//no more fiels is needed
		Json::Value packet;
		packet["DATA"].append(whoAreYou);
		packet["PACKET_HEADER"] = whoAreYou_Packet_Header;
		Json::FastWriter writer;

		return writer.write(packet);
	}

	static std::string makeClientDonePacket() {
		Json::Value header = createPacketHeader(pckt_header("1"));
		Json::Value msg = createMessageHeader(
				msg_header("0", "NS3_SIMULATOR", "CLIENT_MESSAGES_DONE", "SYS"));
		Json::Value packet;
		packet["DATA"].append(msg);
		packet["PACKET_HEADER"] = header;
		Json::FastWriter writer;
		return writer.write(packet);
	}


	static std::string makeWhoAmIPacket(const std::string& token) {
		Json::Value whoAmI_Packet_Header = createPacketHeader(pckt_header("1"));
		Json::Value whoAreYou = createMessageHeader(msg_header("0", "NS3_SIMULATOR", "WHOAMI"));
		whoAreYou["ID"] = "0";
		whoAreYou["token"] = token;
		whoAreYou["TYPE"] = "NS3_SIMULATOR";

		//Required services.
		whoAreYou["REQUIRED_SERVICES"].append("SIMMOB_SRV_TIME");
		whoAreYou["REQUIRED_SERVICES"].append("SIMMOB_SRV_ALL_LOCATIONS");
		whoAreYou["REQUIRED_SERVICES"].append("SIMMOB_SRV_UNKNOWN");

		//no more fiels is needed
		Json::Value packet;
		packet["DATA"].append(whoAreYou);
		packet["PACKET_HEADER"] = whoAmI_Packet_Header;

		Json::FastWriter writer;
		return writer.write(packet);
	}
	//this methd will create a json-formatted packet
	static  void makeGO_ClientArrayElement(unsigned int go, std::vector<unsigned int> clients, Json::Value & output) {
		std::vector<unsigned int>::iterator it(clients.begin()),
				it_end(clients.end());
		Json::Value & GoClientMsg = output;
		GoClientMsg["GO"] = go;
		for(; it != it_end; it++)
		{
			GoClientMsg["CLIENTS"].append(*it);
		}
	}

	//this methd will create a json-formatted packet
	static std::string makeGO_Client(unsigned int go, std::vector<unsigned int> clients) {
		Json::Value GoClientPacketHeader = createPacketHeader(
				pckt_header("1"));
		Json::Value GoClientSMsg = createMessageHeader(
				msg_header("0", "NS3_SIMULATOR", "GOCLIENT"));
		GoClientSMsg["ID"] = "0";
		GoClientSMsg["TYPE"] = "NS3_SIMULATOR";
		std::vector<unsigned int>::iterator it(clients.begin()),
				it_end(clients.end());
		//later, there can be multiple groups
		//specified in a single message.
		//so we send an array in a single message.
		//needless to mention that this message is only 'one' element'
		//in the 'DATA' section of the packet
		Json::Value GoClientMsg;
		GoClientMsg["GO"] = go;
		for(; it != it_end; it++)
		{
			GoClientMsg["CLIENTS"].append(*it);
		}
		//todo, there will be multiple groups created and appended to this section
		GoClientSMsg["GROUPS"].append(GoClientMsg);
		//no more fiels is needed
		Json::Value packet;
		packet["DATA"].append(GoClientMsg);
		packet["PACKET_HEADER"] = GoClientPacketHeader;
		return Json::FastWriter().write(packet);

	}

//	just conveys the tick
	static Json::Value makeTimeData(unsigned int tick, unsigned int elapsedMs) {
		Json::Value time = createMessageHeader(
				msg_header("0", "SIMMOBILITY", "TIME_DATA"));
		time["tick"] = tick;
		time["elapsed_ms"] = elapsedMs;
		return time;
	}

	static std::string makeTimeDataString(unsigned int tick,
			unsigned int elapsedMs) {
		Json::Value time = makeTimeData(tick, elapsedMs);
		Json::FastWriter writer;
		return writer.write(time);
	}

	static std::string makeLocationDataString(int x, int y) {
		Json::Value loc = makeLocationData(x, y);
		Json::FastWriter writer;
		return writer.write(loc);
	}

	static Json::Value makeLocationData(int x, int y) {

		Json::Value loc = createMessageHeader(
				msg_header("0", "SIMMOBILITY", "LOCATION_DATA"));
		loc["x"] = x;
		loc["y"] = y;

		return loc;
	}
	//@originalMessage input
	//@extractedType output
	//@extractedData output
	//@root output
	static bool getMessageTypeAndData(std::string &originalMessage,
			std::string &extractedType, std::string &extractedData,
			Json::Value &root_) {
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(originalMessage, root, false);
		if (not parsedSuccess) {
//			NS_LOG_UNCOND( "Parsing [" << originalMessage << "] Failed");
			return false;
		}
		extractedType = root["MESSAGE_TYPE"].asString();
		//the rest of the message is actually named after the type name
		extractedData = root[extractedType.c_str()].asString();
		root_ = root;
		return true;
	}
};

}

