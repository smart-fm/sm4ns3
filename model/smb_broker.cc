//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_broker.h"
#include "registration.h"
#include "handler_base.h"

#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/ptr.h"

#include <time.h>


using std::string;
using namespace ns3;

unsigned int sm4ns3::Broker::m_global_tick;
unsigned int sm4ns3::Broker::global_pckt_cnt;

NS_LOG_COMPONENT_DEFINE("SimMobility");

sm4ns3::Broker::Broker(const string& simmob_host, const string& simmob_port) :
	conn(io_service, this, simmob_host, simmob_port),
	iorun_thread(ns3::SystemThread(ns3::MakeNullCallback<void>()))
{
	m_pause = true;
	global_pckt_cnt = 0;
	JsonParser::serialize_begin(m_outgoing);
}

sm4ns3::Broker::~Broker() 
{
	io_service.stop();
	iorun_thread.Join();
}


void sm4ns3::Broker::onMessageReceived(const BundleHeader& header, const string& input) 
{
	//What to do with the message now?
	//if you are in the initial stage of being authenticated or getting the
	// pre-simulation configs, you have to act immediately
	// otherwise(if simulation is started) post the messages into the main incoming queue
	CriticalSection lock(mutex_pause);

	//Extract and post message. The Broker's other thread is spinning on m_pause.
	if(!parsePacket(header, input)) {
		std::cout <<"Error: could not parse packet.\n";
	}
	m_pause = false;
}



bool sm4ns3::Broker::start(std::string application) 
{
	m_global_tick = 0;
	//bool res;

	//TODO: Clean this up further
	sm4ns3::Registration* registration = NULL;
	if (application=="Default") {
		registration = new sm4ns3::Registration(this);
	} else if (application=="stk") {
		registration = new sm4ns3::WFD_Registration(this);
	} else {
		throw std::runtime_error("Unknown application.");
	}

	if (!registration->start()) {
		NS_LOG_ERROR( "Registration Failed" );
		return false;
	}

	if (!conn.isOpen()) {
		NS_LOG_ERROR( "Socket is not open" );
		return false;
	}

	NS_LOG_DEBUG( "starting async_read" );

	//start the async operation
	conn.async_receive();

	if (!conn.isOpen()) {
		NS_LOG_ERROR( "Socket is not open" );
		return false;
	}

	//This needs to go in its own thread; perhaps there is a better way of calling it? (We don't really need async access).
	iorun_thread = SystemThread(MakeCallback (&Broker::run_io_service, this));
	iorun_thread.Start();

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


void sm4ns3::Broker::run_io_service() 
{
	io_service.run();
}


sm4ns3::OngoingSerialization& sm4ns3::Broker::getOutgoing()
{
	return m_outgoing;
}

void sm4ns3::Broker::pause() 
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
			if(!conn.isOpen()) {
				std::cout <<"Connection not good; exiting.\n";
				return;
			}

			processIncoming();
			NS_LOG_DEBUG( "Broker::processIncoming() done" );

			sendOutgoing();
			NS_LOG_DEBUG( "Broker::sendOutgoing() done" );

			//Now send the "ClientDone" message.
			//TODO: I am not sure why we don't do this with sendOutgoing.
			sm4ns3::OngoingSerialization res;
			JsonParser::serialize_begin(res);
			sm4ns3::JsonParser::makeClientDone(res);
			std::string resMsg;
			sm4ns3::BundleHeader resHeader;
			JsonParser::serialize_end(res, resHeader, resMsg);

			conn.send(resHeader, resMsg);	//due to nature of DES, this must be sync rather than async
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



void sm4ns3::Broker::processIncoming() 
{
	MessageConglomerate conglom;
	while (m_incoming.pop(conglom)) {
		//Conglomerates contain whole swaths of messages themselves.
		for (int i=0; i<conglom.getCount(); i++) {
			if (NEW_BUNDLES) {
				throw std::runtime_error("processIncoming() for NEW_BUNDLES not yet supported."); 
			} else {
				const Json::Value& jsMsg = conglom.getMessage(i);

				if (!jsMsg.isMember("MESSAGE_TYPE")) {
					std::cout <<"Invalid message, no message_type\n";
					return;
				}

				//Get the handler, let it parse its own expected message type.
				const sm4ns3::Handler* handler = handleLookup.getHandler(jsMsg["MESSAGE_TYPE"].asString());
				if (handler) {
					handler->handle(conglom, i, this);
				} else {
					std::cout <<"no handler for type \"" <<jsMsg["MESSAGE_TYPE"].asString() << "\"\n";
				}
			}
		}
	}
}


void sm4ns3::Broker::sendOutgoing() {
	//Some fiddling.
//	std::vector<Json::Value> messages;
//	Json::Value root;
//	while (m_outgoing.pop(root)) { messages.push_back(root); }

	//Now send.
	std::string msg;
	sm4ns3::BundleHeader head;
	if (!JsonParser::serialize_end(m_outgoing, head, msg))  {
		std::cout <<"Broker couldn't serialize messages.\n";
		return;
	}
	conn.send(head, msg);
	JsonParser::serialize_begin(m_outgoing);
}

//parses the packet to extract messages,
//then redirects the parsed messages based on their
//type and category for further processing
//return value will tell you whether notify or not
bool sm4ns3::Broker::parsePacket(const BundleHeader& header, const std::string &input)
{
	//End of stream reached?
	if(input.empty()){
		std::cout <<"Empty packet; ending simulation.\n";
		return true;
	}

	//Let the serializer handle the heavy lifting.
	MessageConglomerate temp;
	if (!JsonParser::deserialize(header, input, temp)) {
		std::cout <<"Broker couldn't parse packet.\n";
		return false;
	}

	//We have to introspect a little bit, in order to find our READY_TO_RECEIVE message.
	bool res = false;
	for (int i=0; i<temp.getCount(); i++) {
		if (NEW_BUNDLES) {
			throw std::runtime_error("parseAgentsInfo() for NEW_BUNDLES not yet supported."); 
		} else {
			const Json::Value& jsMsg = temp.getMessage(i);
			if (jsMsg.isMember("MESSAGE_TYPE") && jsMsg["MESSAGE_TYPE"] == "READY_TO_RECEIVE") {
				res = true;
				break;
			}
		}
	}

	//New messages to process
	m_incoming.push(temp);

	return res;
}



sm4ns3::Connection& sm4ns3::Broker::getConnection()
{
	return conn;
}


