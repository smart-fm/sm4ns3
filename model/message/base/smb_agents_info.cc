//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_agents_info.h"
#include "smb_agent.h"
#include "smb_broker.h"

sim_mob::MSG_Agents_Info::MSG_Agents_Info(const Json::Value& data_, const sim_mob::msg_header& header) : Message(data_, header)
{
}

void sim_mob::HDL_Agents_Info::handle(msg_ptr message_, Broker* broker) const
{
	NS_LOG_UNCOND( "Broker::processInitMessages::AGENTS_INFO" );
	const Json::Value &data = message_->getData();
	//add
	if (data.isMember("ADD")) {
		if (!data["ADD"].isArray()) {
			NS_LOG_UNCOND( "Array of Agents(to add) information not found");
			return;
		}
		Json::Value add_json = data["ADD"];
		for (unsigned int i = 0; i < add_json.size(); i++) {
			unsigned int ii = add_json[i]["AGENT_ID"].asUInt();
			ns3::Ptr<Agent> agent = ns3::CreateObject<sim_mob::Agent>(ii, broker);
			sim_mob::Agent::AddAgent(ii, agent);
		}
	}

	//remove
	if (data.isMember("REMOVE")) {
		if (!data["REMOVE"].isArray()) {
			NS_LOG_UNCOND( "Array of Agents(to remove) information not found");
			return ;
		}
		Json::Value add_json = data["REMOVE"];
		for (unsigned int i = 0; i < add_json.size(); i++) {
			unsigned int ii = add_json[i]["AGENT_ID"].asUInt();
			sim_mob::Agent::RemoveAgent(ii);
		}
	}

}


