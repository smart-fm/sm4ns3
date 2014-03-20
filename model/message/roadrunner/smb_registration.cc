//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_registration.h"

#include <fstream>
#include <string>

#include "smb_agent.h"
#include "smb_connection.h"
#include "serialize.h"
#include "smb_broker.h"


sm4ns3::Registration::Registration(sm4ns3::Broker* broker_, std::string application_) :
	m_application(application_),
	m_broker(broker_)
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
	while (!m_broker->getConnection().connect()) {
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
	std::string str;
	if (!m_broker->getConnection().receive(str)) {
		std::cout <<"Error reading WHOAREYOU.\n";
		return false;
	}

	//Parse it.
	Json::Value props;
	sm4ns3::MessageBase msg;
	if (!JsonParser::deserialize_single(str, "WHOAREYOU", msg, props)) {
		std::cout <<"Error deserializing WHOAREYOU message.\n";
		return false;
	}

	//Make sure it has a token.
	if (!props.isMember("token")) {
		std::cout <<"ERROR: \"token\" not found\n";
		return false;
	}

	//Prepare a response.
	std::vector<Json::Value> res;
	res.push_back(JsonParser::makeWhoAmI(props["token"].asString()));
	std::string whoami;
	JsonParser::serialize(res, whoami);

	if (!m_broker->getConnection().send(whoami)) {
		std::cout <<"ERROR: unable to send WHOAMI response.\n";
		return false;
	}
	return true;
}

bool sm4ns3::Registration::doAGENTS_INFO()
{
	//A single "AGENTS_INFO" message it expected
	std::string str;
	if (!m_broker->getConnection().receive(str)) {
		std::cout <<"Error reading AGENTS_INFO.\n";
		return false;
	}

	//Parse it.
	Json::Value props;
	sm4ns3::MessageBase msg;
	if (!JsonParser::deserialize_single(str, "AGENTS_INFO", msg, props)) {
		std::cout <<"Error deserializing AGENTS_INFO message.\n";
		return false;
	}

	//Parse it using existing functionality.
	AgentsInfoMessage aInfo = JsonParser::parseAgentsInfo(props);
	if (aInfo.addAgentIds.empty()) {
		std::cout <<"ADD Agents info not found (empty).\n";
		return false;
	}

	//Handle each new Agent ID
	for (std::vector<unsigned int>::const_iterator it=aInfo.addAgentIds.begin(); it!=aInfo.addAgentIds.end(); it++) {
		//TODO: Fix this further.
		if (m_application=="Default") {
			sm4ns3::Agent::AddAgent(*it, ns3::CreateObject<Agent>(*it, m_broker));
		} else if (m_application=="stk") {
			sm4ns3::Agent::AddAgent(*it, ns3::CreateObject<WFD_Agent>(*it, m_broker));
		} else {
			throw std::runtime_error("Invalid m_application.");
		}
	}

	return true;
}

bool sm4ns3::Registration::doInitialization()
{
	for(std::map<unsigned int, ns3::Ptr<Agent> >::const_iterator it = Agent::AllAgents.begin(); it!=Agent::AllAgents.end(); it++){
		it->second->init();
	}
	return true;
}


bool sm4ns3::Registration::doREADY() 
{
	//expect to read READY message
	std::string str;
	if (!m_broker->getConnection().receive(str)) {
		return false;
	}

	//Parse it.
	Json::Value props;
	sm4ns3::MessageBase msg;
	if (!JsonParser::deserialize_single(str, "READY", msg, props)) {
		std::cout <<"Error deserializing READY message.\n";
		return false;
	}

	return true;
}



///////////////////////////////////////////////////////////////
//  WIFI Direct Registration functionality
//  Used for Super Tux Kart
///////////////////////////////////////////////////////////////


sm4ns3::WFD_Registration::WFD_Registration(sm4ns3::Broker* broker_, std::string application_) :
	Registration(broker_, application_)
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
	const std::string msg = makeGO_ClientPacket();

	//Send this information to simmobility
	if (!m_broker->getConnection().send(msg)) {
		return false;
	}

	//Do ns3 protocol settings
	WFD_Agent::configAll();

	return true;
}


//todo: change this sophisticated algorithm :) to support multiple wfd groups in a simulation
bool sm4ns3::WFD_Registration::doRoleAssignment()
{
	std::map<unsigned int,ns3::Ptr<Agent> >::iterator it = sm4ns3::Agent::getAgents().begin();

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



//since we dont want tho include WFD_Group headers to serialize.h(and we dont know why!!!!)
//we define a method here only.
std::string sm4ns3::WFD_Registration::makeGO_ClientPacket()
{
	//First make the single message.
	Json::Value res;
	sm4ns3::JsonParser::addDefaultMessageProps(res, "GOCLIENT");

	//Custom properties.
	res["ID"] = "0";
	res["TYPE"] = "NS3_SIMULATOR";

	//Multi-group formation.
	for(std::map<unsigned int, WFD_Group>::const_iterator it=WFD_Groups_.begin(); it!=WFD_Groups_.end(); it++) {
		Json::Value clientMsg;
		WFD_Registration::makeGO_ClientArrayElement(it->second.GO, it->second.members, clientMsg);
		res["GROUPS"].append(clientMsg);
	}

	//Done
	std::string msg;
	std::vector<Json::Value> vect;
	vect.push_back(res);
	sm4ns3::JsonParser::serialize(vect, msg);
	return msg;
}


void sm4ns3::WFD_Registration::makeGO_ClientArrayElement(unsigned int go, std::vector<unsigned int> clients, Json::Value & output) 
{
	std::vector<unsigned int>::iterator it(clients.begin()),it_end(clients.end());
	Json::Value & GoClientMsg = output;
	GoClientMsg["GO"] = go;
	for(; it != it_end; it++) {
		GoClientMsg["CLIENTS"].append(*it);
	}
}



