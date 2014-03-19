#include "smb_rr_casts.h"
#include "smb_serializer.h"
#include "ns3/packet.h"
#include "smb_agent.h"
#include "smb_broker.h"
#include "smb_serializer.h"

namespace sim_mob {
class Handler;
} //End namespace sim_mob



void sim_mob::roadrunner::HDL_UNICAST::handle(const Json::Value& msg, Broker* broker) const
{
	//Ask the serializer for a Unicast message.
	UnicastMessage ucMsg = JsonParser::parseUnicast(msg);

	//Prepare a message.
	std::string android_sender_id = ucMsg.sender_id;
	std::string android_sender_type = ucMsg.sender_type;
	std::string android_receiver_id = ucMsg.receiver;
	std::string android_receiver_type = ucMsg.sender_type;

	//send to destination
	//TODO: was this ever implemented? ~Seth
}

void sim_mob::roadrunner::HDL_MULTICAST::handle(const Json::Value& msg, Broker* broker) const
{
	//Ask the serializer for a Multicast message.
	MulticastMessage mcMsg = JsonParser::parseMulticast(msg);

	//Prepare and send a message to each client.
	//TODO: Can we invoke the serializer on this somehow?
	Json::Value res;

	//Basic message properties.
	res["SENDER"] = mcMsg.sender_id;
	res["SENDER_TYPE"] = mcMsg.sender_type;
	res["MESSAGE_TYPE"] = mcMsg.msg_type;
	res["MESSAGE_CAT"] = mcMsg.msg_cat;

	//Custom message properties.
	res["SENDING_AGENT"] = mcMsg.sender_id;
	res["MULTICAST_DATA"] = mcMsg.msgData;

	//Retrieve the sending agent.
	ns3::Ptr<Agent> sending_agent = sim_mob::Agent::getAgent(res["SENDING_AGENT"].asUInt());
	if (!sending_agent) {
		std::cout <<"Sending agent is invalid.\n";
		return;
	}

	//Now loop over recipients, setting remaining custom properites and sending the message.
	for (std::vector<unsigned int>::const_iterator it=mcMsg.recipients.begin(); it!=mcMsg.recipients.end(); it++) {
		//Not sure if all of these must be set for every recipient.
		res["RECEIVING_AGENT_ID"] = *it;
		res["GLOBAL_PACKET_COUNT"] = broker->global_pckt_cnt;
		res["TICK_SENT"] = broker->m_global_tick;

		//Retrieve the current recipient.
		ns3::Ptr<Agent> receiving_agent = sim_mob::Agent::getAgent(*it);
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


