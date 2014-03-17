//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_broker.h"
#include "smb_serializer.h"
#include "smb_registration.h"
#include "smb_system_factory.h"
#include "smb_rr_factory.h"

#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/log.h"


#include <time.h>
#include <boost/assign/list_of.hpp>



using namespace ns3;

namespace sim_mob {

unsigned int Broker::m_global_tick;
unsigned int Broker::global_pckt_cnt;

Broker::Broker(std::string simmobility_address, std::string simmobility_port) :
	simmob_host(simmobility_address),
	simmob_port(simmobility_port),
	m_messageReceiveCallback(boost::function<void(std::string)>(boost::bind(&Broker::messageReceiveCallback,this, _1))),m_cnn(m_io_service,m_messageReceiveCallback)
{
//	NS_LOG_UNCOND( "In Broker::Broker()" );
	msgFactorySys.reset(new sim_mob::SYS_MSG_Factory());
	msgFactoryApp.reset(new sim_mob::roadrunner::RR_Factory());

	m_pause = true;
	global_pckt_cnt = 0;
}

Broker::~Broker() {
	m_io_service.stop();
}

bool Broker::start(std::string application) {
	m_global_tick = 0;
//	//debug commenting
	//do the registration
	//sorry a little rework of the factory
	bool res;
	const sim_mob::Registration *registration_ = sim_mob::Registration::getFactory().getPrototype(application,res);
	sim_mob::Registration *registration = registration_->clone();

	if (!registration->start()) {
		NS_LOG_UNCOND( "Registration Failed" );
		return false;
	}

	if (!m_cnn.is_open()) {
		NS_LOG_UNCOND( "Socket is not open" );
	}

	NS_LOG_UNCOND( "starting async_read" );

	//start the async operation
	m_cnn.async_receive();

	if (!m_cnn.is_open()) {
		NS_LOG_UNCOND( "Socket is not open" );
	}

	//start a thread for io_service_run
	//service_thread = SystemThread(MakeCallback (&Broker::run_io_service, this));
	//service_thread.Start();

	//NOTE: There's no reason whatsoever for this to be on its own thread. 
	m_io_service.run();


//	processInitMessages(); //go to function's definition's to see why we call this method here!

	NS_LOG_UNCOND( "basic Network Setting" );

	  // disable fragmentation for frames below 2200 bytes
	  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
	  // turn off RTS/CTS for frames below 2200 bytes
	  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
	  // Fix non-unicast data rate to be the same as that of unicast
	  std::string phyMode ("DsssRate1Mbps");
	  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
	NS_LOG_UNCOND( "Broker::start returning" );
	Simulator::ScheduleNow(&Broker::pause, this);
	return true;
}


void Broker::insertOutgoing(Json::Value &value) {
	m_outgoing.post(value);
}

void Broker::pause() {
	CriticalSection lock(mutex_pause);
	NS_LOG_UNCOND( "pausing the simulation" );
	while(m_pause) {
		cond_sim.Wait();
	}
	m_pause = true;

	NS_LOG_UNCOND( "Simulation UNpaused" );
	processIncoming();
	NS_LOG_UNCOND( "Broker::processIncoming() done" );
	sendOutgoing();
	NS_LOG_UNCOND( "Broker::sendOutgoing() done" );
	m_cnn.send(JsonParser::makeClientDonePacket());	//due to nature of DES, this must be sync rather than async
	NS_LOG_UNCOND( "Broker::send to simmobility done" );
	Simulator::Schedule(MilliSeconds(100), &Broker::pause, this);
	NS_LOG_UNCOND( "Next Tick (" << m_global_tick << ") Scheduling done" );
	m_global_tick++;
}



void Broker::processIncoming() {
	//just pop off the message queue and click handl ;)
	msg_ptr msg;
	while (m_incoming.pop(msg)) {
		hdlr_ptr handler = msg->supplyHandler();
		if (handler) {
			handler->handle(msg, this);
		}
		else
		{
			NS_LOG_UNCOND( "no handler for [" << msg->getData()["MESSAGE_TYPE"].asString() << "]" );
		}
	}

}


void Broker::sendOutgoing() {

	//just pop off the message queue and click handl ;)
	Json::Value root ;
	int i = 0;
	Json::Value msg;
	while (m_outgoing.pop(msg)) {
		root["DATA"].append(msg);
		i++;
	}
	Json::Value header = JsonParser::createPacketHeader(i);
	root["PACKET_HEADER"] = header;
	m_cnn.send(Json::FastWriter().write(root));
}

//parses the packet to extract messages,
//then redirects the parsed messages based on their
//type and category for further processing
//return value will tell you whether notify or not
bool Broker::parsePacket(std::string &input)
{
	bool notify = false;
	std::string type, data;
	Json::Value root;
	sim_mob::pckt_header packetHeader;
	if(!sim_mob::JsonParser::parsePacketHeader(input, packetHeader, root))
	{
		return false;
	}
	if(!sim_mob::JsonParser::getPacketMessages(input,root))
	{
		return false;
	}
//	NS_LOG_UNCOND( "ParsePacket::nof_messages = " << root.size() );
	for (unsigned int index = 0; index < root.size(); index++) {
		msg_header messageHeader;
		msg_data_t & curr_json_msg = root[index];
		if (!sim_mob::JsonParser::parseMessageHeader(curr_json_msg, messageHeader)) {
			continue;
		}
//		NS_LOG_UNCOND( "ParsePacket::" << index << ": (" <<  messageHeader.msg_cat << ":" <<  messageHeader.msg_type << ")" );
		msg_ptr msg = msg_ptr();

		//TODO: Combine message categories, later.
		bool success = false;
		if (messageHeader.msg_cat == "SYS") {
			success = msgFactorySys->createMessage(curr_json_msg,msg);
		} else if (messageHeader.msg_cat == "APP") {
			success = msgFactoryApp->createMessage(curr_json_msg,msg);
		} else { throw std::runtime_error("Unknown message category."); }


		if(success) {
			//this message is essentially the last one in a packet
			if((messageHeader.msg_cat == "SYS") && (messageHeader.msg_type == "READY_TO_RECEIVE")) {
				notify =  true;
			} else {
				m_incoming.post(msg);
			}
		}
	}		//for loop
	return notify;
}

void Broker::messageReceiveCallback(std::string input) {
//	what to do with the message now?
	//if you are in the initial stage of
	//being authenticated or getting the
	//pre-simulation configs, you have to act immediately
	//otherwise(if simulation is started)
	//post the messages into the main incoming queue
	CriticalSection lock(mutex_pause);

	//extract and post message
	if(parsePacket(input) == true) {
		NS_LOG_UNCOND(  "notifying cond_sim");
		m_pause = false;

		cond_sim.Signal();
	}
}

void Broker::setSimmobilityConnectionPoint(std::string simmobility_address,
		std::string simmobility_port) {
	simmob_host = simmobility_address;
	simmob_port = simmobility_port;
}

sim_mob::Connection & Broker::getConnection(){
	return m_cnn;
}

} //namespace
