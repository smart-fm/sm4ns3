//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_broker.h"
#include "smb_serializer.h"
#include "smb_registration.h"
#include "smb_message_handler_base.h"

#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/log.h"

#include <time.h>


using std::string;
using namespace ns3;

namespace sim_mob {

unsigned int Broker::m_global_tick;
unsigned int Broker::global_pckt_cnt;

NS_LOG_COMPONENT_DEFINE("SimMobility");

Broker::Broker(const string& simmobility_address, const string& simmobility_port) :
	simmob_host(simmobility_address),
	simmob_port(simmobility_port),
	conn(io_service, this),
	iorun_thread(ns3::SystemThread(ns3::MakeNullCallback<void>()))
{
	m_pause = true;
	global_pckt_cnt = 0;
}

Broker::~Broker() 
{
	io_service.stop();
	iorun_thread.Join();
}


void Broker::onMessageReceived(const string& input) 
{
	//What to do with the message now?
	//if you are in the initial stage of being authenticated or getting the
	// pre-simulation configs, you have to act immediately
	// otherwise(if simulation is started) post the messages into the main incoming queue
	CriticalSection lock(mutex_pause);

	//Extract and post message
	if(parsePacket(input)) {
		m_pause = false;
//		cond_sim.Signal();
	}
}



bool Broker::start(std::string application) 
{
	m_global_tick = 0;
	//bool res;

	//TODO: Clean this up further
	sim_mob::Registration* registration = NULL;
	if (application=="Default") {
		registration = new sim_mob::Registration(this, simmob_host, simmob_port);
	} else if (application=="stk") {
		registration = new sim_mob::WFD_Registration(this, simmob_host, simmob_port);
	} else {
		throw std::runtime_error("Unknown application.");
	}

	if (!registration->start()) {
		NS_LOG_ERROR( "Registration Failed" );
		return false;
	}

	if (!conn.is_open()) {
		NS_LOG_ERROR( "Socket is not open" );
		return false;
	}

	NS_LOG_DEBUG( "starting async_read" );

	//start the async operation
	conn.async_receive();

	if (!conn.is_open()) {
		NS_LOG_ERROR( "Socket is not open" );
		return false;
	}

	//start a thread for io_service_run
	//service_thread = SystemThread(MakeCallback (&Broker::run_io_service, this));
	//service_thread.Start();

	//This needs to go in its own thread; perhaps there is a better way of calling it? (We don't really need async access).
	iorun_thread = SystemThread(MakeCallback (&Broker::run_io_service, this));
	iorun_thread.Start();


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


void Broker::run_io_service() 
{
	io_service.run();
}


void Broker::insertOutgoing(const Json::Value &value) 
{
	m_outgoing.post(value);
}

void Broker::pause() 
{
	timespec slTm;
	slTm.tv_sec = 0;
	slTm.tv_nsec = 100000; //0.1ms

	//Switched to busy-waiting; Conditions are better, but I want to reduce boost dependencies (and ns3::SystemCondition is flawed).
	for (;;) {
		{
		CriticalSection lock(mutex_pause);
		if (!m_pause) {
			//Time to act.
			m_pause = true;
	
			NS_LOG_DEBUG( "Simulation UNpaused" );

			processIncoming();
			NS_LOG_DEBUG( "Broker::processIncoming() done" );

			sendOutgoing();
			NS_LOG_DEBUG( "Broker::sendOutgoing() done" );
	
			conn.send(JsonParser::makeClientDonePacket());	//due to nature of DES, this must be sync rather than async
			NS_LOG_DEBUG("Broker::send to simmobility done" );

			Simulator::Schedule(MilliSeconds(100), &Broker::pause, this);
			NS_LOG_DEBUG("Next Tick (" << m_global_tick << ") Scheduling done" );
			m_global_tick++;

			break; //Done.
		}
		} //End critical section

		//Else, sleep
		nanosleep(&slTm, NULL);
	}
}



void Broker::processIncoming() 
{
	msg_ptr msg;
	while (m_incoming.pop(msg)) {
		const sim_mob::Handler* handler = msgFactory.getHandler(msg->getHeader().msg_type);
		if (handler) {
			handler->handle(msg, this);
		} else {
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
	conn.send(Json::FastWriter().write(root));
}

//parses the packet to extract messages,
//then redirects the parsed messages based on their
//type and category for further processing
//return value will tell you whether notify or not
bool Broker::parsePacket(const std::string &input)
{
	bool notify = false;
	std::string type, data;
	Json::Value root;
	sim_mob::pckt_header packetHeader;
	if(!sim_mob::JsonParser::parsePacketHeader(input, packetHeader, root)) {
		return false;
	}
	if(!sim_mob::JsonParser::getPacketMessages(input,root)) {
		return false;
	}

	for (unsigned int index = 0; index < root.size(); index++) {
		msg_header messageHeader;
		const Json::Value& curr_json_msg = root[index];
		if (!sim_mob::JsonParser::parseMessageHeader(curr_json_msg, messageHeader)) {
			continue;
		}
		msg_ptr msg = msg_ptr();

		bool success = msgFactory.createSingleMessage(curr_json_msg,msg);


		if(success) {
			//this message is essentially the last one in a packet
			if((messageHeader.msg_cat == "SYS") && (messageHeader.msg_type == "READY_TO_RECEIVE")) {
				notify =  true;
			} else {
				m_incoming.post(msg);
			}
		}
	}
	return notify;
}


void Broker::setSimmobilityConnectionPoint(std::string simmobility_address, std::string simmobility_port) 
{
	simmob_host = simmobility_address;
	simmob_port = simmobility_port;
}

sim_mob::Connection & Broker::getConnection(){
	return conn;
}

} //namespace
