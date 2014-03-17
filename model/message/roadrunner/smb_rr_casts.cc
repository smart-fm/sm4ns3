#include "smb_rr_casts.h"
#include "smb_serializer.h"
#include "ns3/packet.h"
#include "smb_agent.h"
#include "smb_broker.h"
namespace sim_mob {
class Handler;

namespace roadrunner
{
MSG_UNICAST::MSG_UNICAST()
{

}
MSG_UNICAST::MSG_UNICAST(msg_data_t& data_): Message(data_)
{

}

msg_ptr MSG_UNICAST::clone(msg_data_t& data_) {
	return msg_ptr (new MSG_UNICAST(data_));
}

Handler * MSG_UNICAST::newHandler()
{
	return new HDL_UNICAST();
}

void HDL_UNICAST::handle(msg_ptr message_,Broker* broker){
	NS_LOG_UNCOND("Handling unicast...");
	//get the sender and receiver
	Json::Value &data = message_->getData();
	sim_mob::msg_header msg_header_;
	if(!sim_mob::JsonParser::parseMessageHeader(data,msg_header_))
	{
		NS_LOG_UNCOND( "HDL_UNICAST::handle: message header incomplete" );
		return;
	}
	std::string android_sender_id(msg_header_.sender_id) ; //easy read
	std::string android_sender_type(msg_header_.sender_type); //easy read
	std::string android_receiver_id(data["RECEIVER"].asString()) ; //easy read
	std::string android_receiver_type(msg_header_.sender_type); //easy read, (same as sender)


	//send to destination

}

MSG_MULTICAST::MSG_MULTICAST()
{

}

MSG_MULTICAST::MSG_MULTICAST(msg_data_t data_): Message(data_)
{

}

msg_ptr MSG_MULTICAST::clone(msg_data_t& data_) {
	return msg_ptr (new MSG_MULTICAST(data_));
}

Handler * MSG_MULTICAST::newHandler()
{
	return new HDL_MULTICAST();
}

void HDL_MULTICAST::handle(msg_ptr message_,Broker* broker){
	NS_LOG_UNCOND( "Handling a MULTICAST message"  );
//	NS_LOG_UNCOND( "The MULTICASTmessage is" << Json::FastWriter().write(message_->getData()) );
	//step-1 : parse the message to get a list of src/dest agent pairs + the message
	Json::Value &jData = message_->getData();
	try
	{
		unsigned int sender_agentId = jData["SENDING_AGENT"].asUInt();
		ns3::Ptr<Agent> sender_agent = sim_mob::Agent::getAgent(sender_agentId);
		Json::Value receiver_agentIDs = jData["RECIPIENTS"];//easy read
		//want to reuse it.no need of this field
//		jData.removeMember("SENDING_AGENT");
		jData.removeMember("RECIPIENTS");
		//ADD A FIELD TO BE USED BY SIMMOBILITY WHEN THIS MESSAGE IS FORWARDED TO IT THROUGH THE RECEIVING NODE
		jData["RECEIVING_AGENT_ID"] = 0;//FILL IT IN THE FOLLOWING LOOP
		for (unsigned int index = 0; index < receiver_agentIDs.size(); index++) {
			unsigned int receiver_agentId = receiver_agentIDs[index].asUInt();
			jData["RECEIVING_AGENT_ID"] = receiver_agentId;
			jData["GLOBAL_PACKET_COUNT"] = broker->global_pckt_cnt;
			jData["TICK_SENT"] = broker->m_global_tick;
			ns3::Ptr<Agent> receiving_agent = sim_mob::Agent::getAgent(receiver_agentId);
			std::string str = Json::FastWriter().write(jData);
			ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>((uint8_t*) str.c_str(), str.length());
			 if(!sender_agent || !receiving_agent ){
				 NS_LOG_UNCOND("HDL_MULTICAST=>Invalid Agent[send:" << sender_agent << " receive:" << receiving_agent << "]");
			 }
			 else
			 {
					if(sender_agent->SendTo(receiving_agent,packet) != -1){
//						NS_LOG_UNCOND( "Sending from[ " << sender_agent->GetAgentId() << "]  to [" << receiving_agent->GetAgentId() << "]" );
					}
			 }

		}
	}
	catch(std::exception e)
	{
		NS_LOG_UNCOND( e.what() << "[The message doesn't have correct sender/receiver information]" );
	}

	//step-2 : send the message

}//handle()


}/* namespace roadrunner */
} /* namespace sim_mob */
