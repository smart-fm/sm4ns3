#include "smb_all_location.h"
#include "smb_agent.h"
#include "smb_broker.h"

#include "smb_serializer.h"

/*sm4ns3::MSG_All_Location::MSG_All_Location(const Json::Value& data_, const sm4ns3::msg_header& header): Message(data_, header)
{

}
sm4ns3::MSG_All_Location::~MSG_All_Location()
{
}*/


void sm4ns3::HDL_All_Location::handle(const Json::Value& msg, Broker* broker) const
{
	//Ask the serializer for an AllLocations message.
	AllLocationsMessage aInfo = JsonParser::parseAllLocations(msg);

	//Now react accordingly.
	boost::unordered_map<unsigned int, ns3::Ptr<Agent> >& all_agents = Agent::getAgents();
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

sm4ns3::HDL_All_Location::~HDL_All_Location() 
{
}


