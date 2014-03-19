//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <fstream>
#include <string>

#include "smb_registration.h"
#include "smb_agent.h"
#include "smb_connection.h"
#include "smb_serializer.h"
#include "smb_broker.h"

namespace sim_mob {

//sim_mob::BaseFactory<Registration*> Registration::m_appRegFactory;
Registration::Registration(sim_mob::Broker* broker_, std::string simmobility_address_, std::string simmobility_port_, std::string application_) :
	m_application(application_),
	m_broker(broker_),
	m_simmobility_address(simmobility_address_),
	m_simmobility_port(simmobility_port_)
{
	NS_LOG_UNCOND( "Registration\n" );
}

Registration::~Registration() 
{
}


Registration * Registration::clone() const
{
	return new Registration(m_broker,m_simmobility_address,m_simmobility_port);
}

bool Registration::doConnect()
{

	//connect
	int cnn_timeout_seconds = 0;
	while (!m_broker->getConnection().connect(m_simmobility_address, m_simmobility_port)) {
		if (++cnn_timeout_seconds == 200) {
			return false;
		}
		sleep(1);
	}

	return true;
}
bool Registration::doWhoAreYou()
{
	//A single "WHOAREYOU" message it expected
	std::string str;
	if (!m_broker->getConnection().receive(str)) {
		std::cout <<"Error reading WHOAREYOU.\n";
		return false;
	}

	//Parse it.
	Json::Value props;
	sim_mob::MessageBase msg;
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

bool Registration::doAGENTS_INFO()
{
	//A single "AGENTS_INFO" message it expected
	std::string str;
	if (!m_broker->getConnection().receive(str)) {
		std::cout <<"Error reading AGENTS_INFO.\n";
		return false;
	}

	//Parse it.
	Json::Value props;
	sim_mob::MessageBase msg;
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
			sim_mob::Agent::AddAgent(*it, ns3::CreateObject<Agent>(*it, m_broker));
		} else if (m_application=="stk") {
			sim_mob::Agent::AddAgent(*it, ns3::CreateObject<WFD_Agent>(*it, m_broker));
		} else {
			throw std::runtime_error("Invalid m_application.");
		}
	}

	return true;
}

bool Registration::doAgentInit(){
	boost::unordered_map<unsigned int, ns3::Ptr<Agent> >::iterator
	it(Agent::m_all_agents.begin()),
	it_end(Agent::m_all_agents.end());
	for(;it != it_end; it++){
		it->second->init();
	}
	return true;
}
//waits for ready message
bool Registration::doREADY() {
	//expect to read READY message
	std::string str;
	if (!m_broker->getConnection().receive(str)) {
		return false;
	}

	//Parse it.
	Json::Value props;
	sim_mob::MessageBase msg;
	if (!JsonParser::deserialize_single(str, "READY", msg, props)) {
		std::cout <<"Error deserializing READY message.\n";
		return false;
	}

	return true;
}


bool Registration::start() {
	NS_LOG_UNCOND("Starting Normal Registration");
	if(!doConnect()) { return false;}
	if(!doWhoAreYou()) { return false;}
	sleep(1); //The way we buffer messages, reading from the socket can return the old WHOAMI message if AGENTSINFO hasn't been sent yet. So just give it some time.
	if(!doAGENTS_INFO()) { return false;}
	if(!doAgentInit()) { return false;}
	if(!doREADY()) { return false;}

	return true;
}

/*******************************************************
 * ***************WIFI Direct Registration *************
 * ***************Used for Super Tux Kart **************
 *******************************************************/
WFD_Registration::WFD_Registration(
		sim_mob::Broker* broker_,
		std::string simmobility_address_,
		std::string simmobility_port_,
		std::string application_
		):
		Registration(broker_,simmobility_address_,simmobility_port_,application_)

{
	NS_LOG_UNCOND( "WFD_Registration\n" );

}

WFD_Registration::~WFD_Registration() {
	// TODO Auto-generated destructor stub
}

Registration * WFD_Registration::clone()const{
	//nothing to configure for now
	return new WFD_Registration(m_broker,m_simmobility_address,m_simmobility_port);
}

//todo: change this sophisticated algorithm :) to support multiple wfd groups in a simulation
bool WFD_Registration::doRoleAssignment(){
	///A very very very very and again
	//VERY sophisticated algorithm to
	///select the Group Owner.
	///Feel free to stop reviewing this
	///method if it is too complex for you.
	boost::unordered_map<unsigned int,ns3::Ptr<Agent> >::iterator it, it_begin, it_end;
	it = it_begin = sim_mob::Agent::getAgents().begin();
	it_end = sim_mob::Agent::getAgents().end();
	//VERY primitive way that will:
	//1-choose the first node in the list as GO
	//2-create ONLY one group.
	WFD_Group wfd;
	wfd.GO = wfd.groupId = it->second->GetAgentId();
	for( ; it != sim_mob::Agent::getAgents().end(); it++){
		wfd.members.push_back(it->second->GetAgentId());
		WFD_Membership[it->second->GetAgentId()] = wfd.groupId;
	}
	WFD_Groups_[wfd.groupId] = wfd;

	return true;
}





//since we dont want tho include WFD_Group headers to serialize.h(and we dont know why!!!!)
//we define a method here only.
std::string WFD_Registration::makeGO_ClientPacket(){
	//First make the single message.
	Json::Value res;
	sim_mob::JsonParser::addDefaultMessageProps(res, "GOCLIENT");

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
	sim_mob::JsonParser::serialize(vect, msg);
	return msg;
}


void sim_mob::WFD_Registration::makeGO_ClientArrayElement(unsigned int go, std::vector<unsigned int> clients, Json::Value & output) 
{
	std::vector<unsigned int>::iterator it(clients.begin()),it_end(clients.end());
	Json::Value & GoClientMsg = output;
	GoClientMsg["GO"] = go;
	for(; it != it_end; it++) {
		GoClientMsg["CLIENTS"].append(*it);
	}
}


/*
 * step-1 do the role assignment by filling some containers local to this class
 * step-2 set proper variable at each agent telling him its role + fill in the corresponding GO or Client Node container
 * step-3 prepare a packet in json format about all agents group information.
 * step-4 send json string to simmobility
 * step-5 do ns3 protocol settings
 */

bool WFD_Registration::WFD_Configuration(){
	//	step-1 do the role assignment by filling some containers local to this class
	if(!doRoleAssignment()) { return false;}
	//	step-2 set proper variable at each agent telling him its role + fill in the corresponding GO or Client Node container
	if(!doAgentInit()) { return false;}
	NS_LOG_UNCOND("doAgentInit-done");
	//	step-3 prepare a packet in json format about all agents group information.
	const std::string msg = makeGO_ClientPacket();
	NS_LOG_UNCOND("makeGO_ClientPacket-done\n'" << msg << "'\n");
	//	step-4-send this information to simmobility
	if (!m_broker->getConnection().send(msg)) {
		return false;
	}
	//	step-5 do ns3 protocol settings
	WFD_Agent::configAll();

	return true;
}

bool WFD_Registration::start() {
	NS_LOG_UNCOND("Starting WFD Registration");
	if(!doConnect()) { return false;}
	NS_LOG_UNCOND("WFD Registration-connect");
	if(!doWhoAreYou()) { return false;}
	NS_LOG_UNCOND("WFD Registration-whoareyou");
	if(!doAGENTS_INFO()) { return false;}
	NS_LOG_UNCOND("WFD Registration-agentInfo");
	if(!WFD_Configuration()) { return false;}
	NS_LOG_UNCOND("WFD_Configuration");
	if(!doREADY()) { return false;}
	NS_LOG_UNCOND("WFD Registration-ready");

	return true;
}

} /* namespace sim_mob */
