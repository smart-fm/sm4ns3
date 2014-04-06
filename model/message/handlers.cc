//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "handlers.h"

#include "ns3/ptr.h"

#include "messages.h"
#include "serialize.h"
#include "smb_agent.h"
#include "smb_broker.h"


sm4ns3::HandlerLookup::HandlerLookup() 
{
	//Register all known handlers.
	HandlerMap["all_locations"] = new sm4ns3::AllLocationHandler();
	HandlerMap["new_agents"] = new sm4ns3::AgentsInfoHandler();
	HandlerMap["ticked_simmob"] = new sm4ns3::NullHandler();
	HandlerMap["opaque_send"] = new sm4ns3::OpaqueSendHandler();

	//Avoid obsolete messages.
	HandlerMap["MULTICAST"] = new sm4ns3::InvalidHandler();
	HandlerMap["UNICAST"] = new sm4ns3::InvalidHandler();
}

sm4ns3::HandlerLookup::~HandlerLookup() 
{
	//Reclaim handlers
	for (std::map<std::string, const sm4ns3::Handler*>::const_iterator it=HandlerMap.begin(); it!=HandlerMap.end(); it++) {
		delete it->second;
	}
	HandlerMap.clear();
}


const sm4ns3::Handler* sm4ns3::HandlerLookup::getHandler(const std::string& msgType)
{
	std::map<std::string, const sm4ns3::Handler*>::const_iterator it = HandlerMap.find(msgType);
	if (it!=HandlerMap.end()) {
		return it->second;
	}

	throw std::runtime_error("Unknown handler for message type.");
}


void sm4ns3::AgentsInfoHandler::handle(const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for an AgentsInfo message.
	AgentsInfoMessage aInfo = JsonParser::parseNewAgents(messages, msgNumber);

	//Process add/remove agent requestss
	for (std::vector<unsigned int>::const_iterator it=aInfo.addAgentIds.begin(); it!=aInfo.addAgentIds.end(); it++) {
		ns3::Ptr<Agent> agent = ns3::CreateObject<sm4ns3::Agent>(*it, broker);
		agent->init();
		sm4ns3::Agent::AddAgent(*it, agent);
	}
	for (std::vector<unsigned int>::const_iterator it=aInfo.remAgentIds.begin(); it!=aInfo.remAgentIds.end(); it++) {
		sm4ns3::Agent::RemoveAgent(*it);
	}
}


void sm4ns3::AllLocationHandler::handle(const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for an AllLocations message.
	AllLocationsMessage aInfo = JsonParser::parseAllLocations(messages, msgNumber);

	//Now react accordingly.
	std::map<unsigned int, ns3::Ptr<Agent> >& all_agents = Agent::getAgents();
	for (std::map<unsigned int, sm4ns3::DPoint>::const_iterator it=aInfo.agentLocations.begin(); it!=aInfo.agentLocations.end(); it++) {
		if(all_agents.find(it->first) == all_agents.end()) {
			std::cout <<"Agent id (" <<it->first << ") not found.\n";
			continue;
		}

		ns3::Ptr<Agent> agent = all_agents[it->first];
		ns3::Vector v(it->second.x, it->second.y, 0.0);
		if(!agent){
			std::cout <<"Agent id (" <<it->first << ") invalid.\n";
			continue;
		} else {
			agent->SetPosition(v);
		}
	}
}



void sm4ns3::OpaqueSendHandler::handle(const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for an OpaqueSendHandler message.
	OpaqueSendMessage osMsg = JsonParser::parseOpaqueSend(messages, msgNumber);

	//Prepare and send a message to each client.
	//TODO: This is a weird case of using JSON as an internal message format.
	//      We should switch this to using an OpaqueReceive message (which can still be converted to JSON).
	Json::Value res;

	//Basic message properties.
	res["msg_type"] = "opaque_receive";

	//Custom message properties.
	res["from_id"] = osMsg.fromId;
	res["data"] = osMsg.data;

	//Retrieve the sending agent.
	ns3::Ptr<Agent> sending_agent = sm4ns3::Agent::getAgent(res["from_id"].asUInt());
	if (!sending_agent) {
		std::cout <<"Sending agent is invalid.\n";
		return;
	}

	//Now loop over recipients, setting remaining custom properites and sending the message.
	for (std::vector<std::string>::const_iterator it=osMsg.toIds.begin(); it!=osMsg.toIds.end(); it++) {
		//Set the "to_id" for this recipient.
		res["to_id"] = *it;

		//TODO: Do we really need these? ~Seth
		//res["GLOBAL_PACKET_COUNT"] = broker->global_pckt_cnt;
		//res["TICK_SENT"] = broker->m_global_tick;

		//Retrieve the current recipient.
		ns3::Ptr<Agent> receiving_agent = sm4ns3::Agent::getAgent(res["to_id"].asUInt());
		if (!receiving_agent) {
			std::cout <<"Receiving agent is invalid.\n";
			continue;
		}

		//Serialize and send the message.
		std::string str = Json::FastWriter().write(res);
		ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>((uint8_t*) str.c_str(), str.length());
		if (sending_agent->SendTo(receiving_agent,packet)==-1) {
			std::cout <<"ERROR: Message could not be sent.\n";
		}
	}
}


