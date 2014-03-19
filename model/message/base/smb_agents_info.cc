//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_agents_info.h"
#include "smb_agent.h"
#include "smb_broker.h"
#include "smb_serializer.h"

/*sim_mob::MSG_Agents_Info::MSG_Agents_Info(const Json::Value& data_, const sim_mob::msg_header& header) : Message(data_, header)
{
}*/

void sim_mob::HDL_Agents_Info::handle(const Json::Value& msg, Broker* broker) const
{
	//Ask the serializer for an AgentsInfo message.
	AgentsInfoMessage aInfo = JsonParser::parseAgentsInfo(msg);

	//Process add/remove agent requestss
	for (std::vector<unsigned int>::const_iterator it=aInfo.addAgentIds.begin(); it!=aInfo.addAgentIds.end(); it++) {
		ns3::Ptr<Agent> agent = ns3::CreateObject<sim_mob::Agent>(*it, broker);
		sim_mob::Agent::AddAgent(*it, agent);
	}
	for (std::vector<unsigned int>::const_iterator it=aInfo.remAgentIds.begin(); it!=aInfo.remAgentIds.end(); it++) {
		sim_mob::Agent::RemoveAgent(*it);
	}
}


