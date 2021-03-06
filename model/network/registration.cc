//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "registration.h"

#include <fstream>
#include <string>

#include "smb_agent.h"
#include "smb_connection.h"
#include "serialize.h"
#include "smb_broker.h"
#include "bundle_version.h"


sm4ns3::Registration::Registration(BrokerBase* broker, std::string app) :
	m_application(app), broker(broker)
{
}

sm4ns3::Registration::~Registration() 
{
}


bool sm4ns3::Registration::start()
{
	std::cout <<"Starting Registration\n";
	if(!doConnect(200)) { return false;}
	if(!doWhoAreYou()) { return false;}

	//The way we buffer messages, reading from the socket can return the old WHOAMI message if AGENTSINFO hasn't been sent yet. So just give it some time.
	sleep(1);

	if(!doAGENTS_INFO()) { return false;}
	if(!doInitialization()) { return false;}
	if(!doREADY()) { return false;}

	return true;
}


bool sm4ns3::Registration::doConnect(unsigned int timeout)
{
	//connect
	unsigned int sec = 0;
	while (!broker->getConnection().connect()) {
		if (sec == 0) {
			std::cout <<"Couldn't connect; trying again for " <<timeout <<" seconds.\n";
		}
		if (++sec == timeout) {
			return false;
		}
		sleep(1);
	}

	std::cout <<"Connected to server.\n";
	return true;
}

bool sm4ns3::Registration::doWhoAreYou()
{
	//A single "WHOAREYOU" message it expected
	BundleHeader head;
	std::string str;
	if (!broker->getConnection().receive(head, str)) {
		std::cout <<"Error reading WHOAREYOU.\n";
		return false;
	}

	//Parse it.
	sm4ns3::MessageConglomerate conglom;
	sm4ns3::MessageBase msg;
	if (!JsonParser::deserialize_single(head, str, "id_request", msg, conglom)) {
		std::cout <<"Error deserializing WHOAREYOU message.\n";
		return false;
	}

	//TODO: Move this into the Serializer class.
	const Json::Value& props = conglom.getJsonMessage(0);
	if (props.isNull()) {
		throw std::runtime_error("Can't deserialize binary id_request (in registration)."); 
	}

	//Make sure it has a token.
	if (!props.isMember("token")) {
		std::cout <<"ERROR: \"token\" not found\n";
		return false;
	}

	//Prepare a response.
	sm4ns3::OngoingSerialization res;
	JsonParser::serialize_begin(res);
	JsonParser::makeIdResponse(res, props["token"].asString());

	std::string whoami;
	JsonParser::serialize_end(res, head, whoami);

	if (!broker->getConnection().send(head, whoami)) {
		std::cout <<"ERROR: unable to send WHOAMI response.\n";
		return false;
	}
	return true;
}

bool sm4ns3::Registration::doAGENTS_INFO()
{
	//A single "AGENTS_INFO" message it expected
	BundleHeader head;
	std::string str;
	if (!broker->getConnection().receive(head, str)) {
		std::cout <<"Error reading AGENTS_INFO.\n";
		return false;
	}

	//Parse it.
	sm4ns3::MessageConglomerate conglom;
	sm4ns3::MessageBase msg;
	if (!JsonParser::deserialize_single(head, str, "new_agents", msg, conglom)) {
		std::cout <<"Error deserializing AGENTS_INFO message.\n";
		return false;
	}

	//TODO: Move this into the Serializer class.
	const Json::Value& props = conglom.getJsonMessage(0);
	if (props.isNull()) {
		throw std::runtime_error("Can't deserialize binary new_agents (in registration)."); 
	}

	//Parse it using existing functionality.
	AgentsInfoMessage aInfo = JsonParser::parseNewAgents(conglom, 0);
	if (aInfo.addAgentIds.empty()) {
		std::cout <<"ADD Agents info not found (empty).\n";
		return false;
	}

	//Handle each new Agent ID
	for (std::vector<std::string>::const_iterator it=aInfo.addAgentIds.begin(); it!=aInfo.addAgentIds.end(); it++) {
		//TODO: Fix this further.
		if (m_application=="Default") {
			sm4ns3::Agent::AddAgent(*it, ns3::CreateObject<Agent>(*it, broker));
		} else if (m_application=="stk") {
			sm4ns3::Agent::AddAgent(*it, ns3::CreateObject<WFD_Agent>(*it, broker));
		} else {
			throw std::runtime_error("Invalid m_application.");
		}
	}

	return true;
}

bool sm4ns3::Registration::doInitialization()
{
	for(std::map<std::string, ns3::Ptr<Agent> >::const_iterator it = Agent::AllAgents.begin(); it!=Agent::AllAgents.end(); it++){
		it->second->init();
	}
	return true;
}


bool sm4ns3::Registration::doREADY() 
{
	//expect to read READY message
	BundleHeader head;
	std::string str;
	if (!broker->getConnection().receive(head, str)) {
		return false;
	}

	//Parse it.
	sm4ns3::MessageConglomerate conglom;
	sm4ns3::MessageBase msg;
	if (!JsonParser::deserialize_single(head, str, "id_ack", msg, conglom)) {
		std::cout <<"Error deserializing READY message.\n";
		return false;
	}

	return true;
}



///////////////////////////////////////////////////////////////
//  WIFI Direct Registration functionality
//  Used for Super Tux Kart
///////////////////////////////////////////////////////////////


sm4ns3::WFD_Registration::WFD_Registration(BrokerBase* broker, std::string app) :
	Registration(broker, app)
{
}

sm4ns3::WFD_Registration::~WFD_Registration() 
{
}


bool sm4ns3::WFD_Registration::doInitialization()
{
	//Do the role assignment by filling some containers local to this class
	if(!doRoleAssignment()) { return false;}

	//Set proper variable at each agent telling him its role + fill in the corresponding GO or Client Node container
	if(!Registration::doInitialization()) { return false;}

	//Prepare a packet in json format about all agents group information.
	sm4ns3::OngoingSerialization res;
	JsonParser::serialize_begin(res);
	sm4ns3::JsonParser::makeGoClient(res, WFD_Groups_);
	std::string msg;
	sm4ns3::BundleHeader head;
	JsonParser::serialize_end(res, head, msg);

	//Send this information to simmobility
	if (!broker->getConnection().send(head, msg)) {
		return false;
	}

	//Do ns3 protocol settings
	WFD_Agent::configAll();

	return true;
}


//todo: change this sophisticated algorithm :) to support multiple wfd groups in a simulation
bool sm4ns3::WFD_Registration::doRoleAssignment()
{
	std::map<std::string, ns3::Ptr<Agent> >::iterator it = sm4ns3::Agent::getAgents().begin();

	//VERY primitive way that will:
	//1-choose the first node in the list as GO
	//2-create ONLY one group.
	WFD_Group wfd;
	wfd.GO = wfd.groupId = it->second->GetAgentId();
	for( ; it != sm4ns3::Agent::getAgents().end(); it++) {
		wfd.members.push_back(it->second->GetAgentId());
		WFD_Membership[it->second->GetAgentId()] = wfd.groupId;
	}
	WFD_Groups_[wfd.groupId] = wfd;

	return true;
}


