#include "smb_all_location.h"
//#include "smb_message_info.h"
#include "smb_agent.h"
#include "smb_broker.h"
namespace sim_mob {
MSG_All_Location::MSG_All_Location(const Json::Value& data_, const sim_mob::msg_header& header): Message(data_, header)
{

}

/*MSG_All_Location::MSG_All_Location()
{
}*/

MSG_All_Location::~MSG_All_Location()
{

}

/*Handler * MSG_All_Location::newHandler()
{
	return new HDL_All_Location();
}*/

void HDL_All_Location::handle(msg_ptr message_,Broker* broker) const
{
	boost::unordered_map<unsigned int, ns3::Ptr<Agent> > &all_agents = Agent::getAgents();
	const Json::Value &data = message_->getData();
	Json::FastWriter w;
//	NS_LOG_UNCOND( w.write(data) );
	Json::Value locationArray = data["LOCATIONS"];
	int n = locationArray.size();
	for(int i= 0; i < n; i++)
	{
		unsigned int id = locationArray[i]["ID"].asUInt();
		if(all_agents.find(id) == all_agents.end())
		{
			NS_LOG_UNCOND( "Agent id[" << id << "] not found" );
			continue;
		}
		double x = locationArray[i]["x"].asDouble();
		double y = locationArray[i]["y"].asDouble();
		 ns3::Ptr<Agent> agent = all_agents[id];
		 ns3::Vector v(x, y, 0.0);
		 if(!agent){
			 NS_LOG_UNCOND("HDL_All_Location=>Invalid Agent[" << id << "]");
		 }
		 else
		 {
			 agent->SetPosition(v);
		 }
	}
}//handle()

HDL_All_Location::~HDL_All_Location() {

}
}//namespace
