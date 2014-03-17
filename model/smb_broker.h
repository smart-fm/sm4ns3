//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "smb_message_queue.h"
#include "smb_message_factory_base.h"
#include "smb_connection.h"

#include <iostream>

#include "ns3/system-mutex.h"
#include "ns3/system-condition.h"

#include <boost/asio/io_service.hpp>
#include <jsoncpp/json/json.h>



namespace sim_mob {

class Broker {
public:
	Broker(std::string simmobility_address = "localhost",std::string simmobility_port = "6745");
	virtual ~Broker();

private:
	//Connection settings; host/port
	std::string simmob_host;
	std::string simmob_port;


	boost::function<void(std::string)> m_messageReceiveCallback;
	sim_mob::Connection m_cnn;


	ns3::SystemMutex mutex_pause;

	ns3::SystemCondition cond_sim;

	bool m_pause;
	sim_mob::MessageQueue<msg_ptr> m_incoming;
	sim_mob::MessageQueue<std::string> m_incoming_conf;
	sim_mob::MessageQueue<Json::Value> m_outgoing;


	//TODO: Clean this up later.
	boost::shared_ptr<sim_mob::MessageFactory<std::string&, msg_ptr, msg_data_t> > msgFactorySys;
	boost::shared_ptr<sim_mob::MessageFactory<std::string&, msg_ptr, msg_data_t> > msgFactoryApp;
public:
	static unsigned int m_global_tick;
	static unsigned int global_pckt_cnt;
	boost::asio::io_service m_io_service;
	virtual bool start(std::string application = "stk");
	void insertOutgoing(Json::Value & value);
	virtual void pause();
	bool processInitMessages();
	void sendClientDone();
	void processIncoming();
	void sendOutgoing();
	bool parsePacket(std::string &input);
	void messageReceiveCallback(std::string message);
	void setSimmobilityConnectionPoint(std::string,std::string);

	sim_mob::Connection & getConnection();
};

} //namespace
