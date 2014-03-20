//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <iostream>

#include "ns3/system-mutex.h"
#include "ns3/system-thread.h"

#include <boost/asio/io_service.hpp>
#include <jsoncpp/json/json.h>

#include "thread_safe_queue.h"
#include "smb_message_factory_base.h"
#include "smb_connection.h"



namespace sm4ns3 {

///Contains various callback functions, allowing other classes to interact with Brokers without requiring the entire object.
class BrokerBase {
public:
	virtual void onMessageReceived(const std::string& message) = 0;
};


class Broker : public BrokerBase{
public:
	Broker(const std::string& simmobility_address="localhost", const std::string& simmobility_port="6745");
	virtual ~Broker();

	//Overriding BrokerBase
	virtual void onMessageReceived(const std::string& message);

private:
	//Connection settings; host/port
	std::string simmob_host;
	std::string simmob_port;

	sm4ns3::Connection conn;


	ns3::SystemMutex mutex_pause;

//	ns3::SystemCondition cond_sim;

	bool m_pause;


	//Since both threads (network+main) interact with these message buffers, we use a ThreadSafeQueue to safely access them.
	//sm4ns3::MessageQueue<msg_ptr> m_incoming;
//	std::vector<Json::Value> m_incoming;
	ThreadSafeQueue<Json::Value> m_incoming;

	//sm4ns3::MessageQueue<Json::Value> m_outgoing;
//	std::vector<Json::Value> m_outgoing;
	ThreadSafeQueue<Json::Value> m_outgoing;

//	sm4ns3::ThreadSafeQueue<std::string> m_incoming_conf;


	sm4ns3::MessageFactory msgFactory;

	//I/O needs to go in its own thread.
	ns3::SystemThread iorun_thread;

	void run_io_service();
public:
	static unsigned int m_global_tick;
	static unsigned int global_pckt_cnt;
	boost::asio::io_service io_service;
	virtual bool start(std::string application = "stk");
	void insertOutgoing(const Json::Value & value);
	virtual void pause();
	bool processInitMessages();
	void sendClientDone();
	void processIncoming();
	void sendOutgoing();
	bool parsePacket(const std::string &input);
	void setSimmobilityConnectionPoint(std::string,std::string);

	sm4ns3::Connection & getConnection();
};

} //namespace
