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
Registration::Registration(
		sim_mob::Broker* broker_,
		std::string simmobility_address_,
		std::string simmobility_port_, std::string application_
		)
:
				m_application(application_),
				m_broker(broker_),
				m_simmobility_address(simmobility_address_),
				m_simmobility_port(simmobility_port_)
{
	// TODO Auto-generated constructor stub
	NS_LOG_UNCOND( "Registration\n" );
}

Registration::~Registration() {
	// TODO Auto-generated destructor stub
}

//USED TO PARSE SINGLE MESSAGE PACKETS
bool Registration::get_message_header(std::string &input,
		msg_header &messageHeader) {

	std::string type, data;
	Json::Value root;
	sim_mob::pckt_header packetHeader;
	if (!sim_mob::JsonParser::parsePacketHeader(input, packetHeader,
			root)) {
		NS_LOG_UNCOND( "Simmobility not Ready, Trying Again" );
		return false;
	}
	if (!sim_mob::JsonParser::getPacketMessages(input, root)) {
		return false;
	}

	int i = 0;
	if (!sim_mob::JsonParser::parseMessageHeader(root[i], messageHeader)) {
		return false;
	}
	return true;
}
Registration * Registration::clone()const{
	return new Registration(m_broker,m_simmobility_address,m_simmobility_port);
}

/*sim_mob::BaseFactory<Registration*> &Registration::getFactory(){
	return m_appRegFactory;
}*/

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
	//expect to read WHOAREYOU message
	std::string WHOAREYOU;
	if (!m_broker->getConnection().receive(WHOAREYOU)) {
		NS_LOG_UNCOND( "read WHOAREYOU error" );
		return false;
	}
	msg_header messageHeader;
	int res = get_message_header(WHOAREYOU, messageHeader);
	if (!res || messageHeader.msg_type != "WHOAREYOU") {
		NS_LOG_UNCOND(
				 "Simmobility didn't send the WHOAREYOU message"
				<< "[" << messageHeader.msg_type << "]" );
		return false;
	}

	Json::Value root;
	sim_mob::pckt_header packetHeader;
	if (!sim_mob::JsonParser::parsePacketHeader(WHOAREYOU, packetHeader, root)) {
		NS_LOG_UNCOND( "Simmobility didn't send a valid packet" );
		return false;
	}
	if (!sim_mob::JsonParser::getPacketMessages(WHOAREYOU, root)) {
		NS_LOG_UNCOND("No messages in this packet" );
		return false;
	}

	Json::Value& curr_json = root[0];
	if (!curr_json.isMember("token")) {
		NS_LOG_UNCOND( "\"token\" not found");
		return false;
	}

	//write whoareyou message
	std::string WHOAMI = JsonParser::makeWhoAmIPacket(curr_json["token"].asString());
	if (!m_broker->getConnection().send(WHOAMI)) {
		return false;
	}
	return true;
}

bool Registration::doAGENTS_INFO()
{
	std::string AGENTS_INFO;
	if (!m_broker->getConnection().receive(AGENTS_INFO)) {
		NS_LOG_UNCOND( "read AGENTS_INFO error" );
		return false;
	}

	std::string type, data;
	Json::Value root;
	sim_mob::pckt_header packetHeader;
	if (!sim_mob::JsonParser::parsePacketHeader(AGENTS_INFO, packetHeader,
			root)) {
		NS_LOG_UNCOND( "Simmobility didn't send a valid packet" );
		return false;
	}
	if (!sim_mob::JsonParser::getPacketMessages(AGENTS_INFO, root)) {
		return false;
	}

	int i = 0;
	msg_header messageHeader;
	if (!sim_mob::JsonParser::parseMessageHeader(root[i], messageHeader)) {
		return false;
	}

	int res = get_message_header(AGENTS_INFO, messageHeader);
	if (!res || messageHeader.msg_type != "AGENTS_INFO") {
		NS_LOG_UNCOND(
				 "Simmobility didn't send the AGENTS_INFO message"
				<< "[" << messageHeader.msg_type << "]" );
		return false;
	}
	Json::Value & curr_json = root[i];

	if (!curr_json.isMember("ADD")) {
		NS_LOG_UNCOND( "\"ADD\" Agents information not found"
				);
		return false;
	}
	if (!curr_json["ADD"].isArray()) {
		NS_LOG_UNCOND( "Array of Agents information not found"
				);
		return false;
	}
	Json::Value add_json = curr_json["ADD"];
	NS_LOG_UNCOND( "ADDing Agents");
	for (unsigned int i = 0; i< add_json.size(); i++) {
		if(!add_json[i].isMember("AGENT_ID")) {
			NS_LOG_UNCOND("AGENT_ID is not a member");
			continue;//actually should throw error
		}
		unsigned int ii = add_json[i]["AGENT_ID"].asUInt();
		if(add_json[i].isMember("AGENT_TYPE")){
			unsigned int type = add_json[i]["AGENT_TYPE"].asUInt();
			NS_LOG_UNCOND(type);
		}
//		bool res;
		NS_LOG_UNCOND("Getting Base Agent of Type " << m_application);
		 //sim_mob::Agent *agentBase = sim_mob::Agent::getFactory().getPrototype(m_application, res);
//		 if(!res){
//			 NS_ASSERT_MSG(res,"Getting the Base Agent from Agent Factory Failed");
//		 }

		//TODO: Fix this further.
		if (m_application=="Default") {
			sim_mob::Agent::AddAgent(ii, ns3::CreateObject<Agent>(ii, m_broker));
		} else if (m_application=="stk") {
			sim_mob::Agent::AddAgent(ii, ns3::CreateObject<WFD_Agent>(ii, m_broker));
		} else {
			throw std::runtime_error("Invalid m_application.");
		}

//		agent->init();
		NS_LOG_UNCOND( "Agent " << ii << " created" );
	}
	NS_LOG_UNCOND( "\n");

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
	std::string READY;
	if (!m_broker->getConnection().receive(READY)) {
		return false;
	}

	msg_header messageHeader;
	int res = get_message_header(READY, messageHeader);
//	NS_LOG_UNCOND( "reg::doReady::received[" << messageHeader.msg_type << "]" );
	if (!res || messageHeader.msg_type != "READY") {
		NS_LOG_UNCOND( "Simmobility didn't send the READY message" );
		return false;
	}
	else{
		NS_LOG_UNCOND( "Simmobility sent the READY message" );
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
const std::string & WFD_Registration::makeGO_ClientPacket(){
	Json::Value GoClientPacketHeader = JsonParser::createPacketHeader(
				pckt_header("1"));
		Json::Value GoClientSMsg = JsonParser::createMessageHeader(
				msg_header("0", "NS3_SIMULATOR", "GOCLIENT"));
		GoClientSMsg["ID"] = "0";
		GoClientSMsg["TYPE"] = "NS3_SIMULATOR";
		std::map<unsigned int, WFD_Group>::iterator it(WFD_Groups_.begin()),
				it_end(WFD_Groups_.end());
		Json::Value GoClientMsg;
		//supports multi group formation
		for(; it != it_end; it++)
		{
			JsonParser::makeGO_ClientArrayElement(it->second.GO, it->second.members,GoClientMsg);
			GoClientSMsg["GROUPS"].append(GoClientMsg);
		}
		//no more fiels is needed
		Json::Value packet;
		packet["DATA"] = GoClientSMsg; //"DATA" : ["GROUPS":["GO":xxx , "CLIENTS":[aaa,bbb,ccc]], ["GO":xxx , "CLIENTS":[aaa,bbb,ccc]],["GO":xxx , "CLIENTS":[aaa,bbb,ccc]]]
		packet["PACKET_HEADER"] = GoClientPacketHeader;
		const std::string & msg = Json::FastWriter().write(packet);
		return msg;
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
