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
#include "handlers.h"
#include "smb_connection.h"



namespace sm4ns3 {

///Contains various callback functions, allowing other classes to interact with Brokers without requiring the entire object.
class BrokerBase {
public:
	///Used by Connection to send messagse back to the Broker. Called asynchronously; make sure to lock properly.
	virtual void onMessageReceived(const std::string& message) = 0;

	///Used by Registration and WFD_Registration
	virtual Connection& getConnection() = 0;

	//Used by Agent, WFD_Agent
	virtual void insertOutgoing(const Json::Value & value) = 0;
};


class Broker : public BrokerBase{
public:
	Broker(const std::string& simmob_host="localhost", const std::string& simmob_port="6745");
	virtual ~Broker();

	//Overriding BrokerBase
	virtual void onMessageReceived(const std::string& message);
	virtual Connection& getConnection();
	virtual void insertOutgoing(const Json::Value & value);

private:
	//Connection to Sim Mobility.
	sm4ns3::Connection conn;

	//This variable+mutex is used to advance the program between reading from a network stream and performing simulation.
	ns3::SystemMutex mutex_pause;
	bool m_pause;

	//Since both threads (network+main) interact with these message buffers, we use a ThreadSafeQueue to safely access them.
	ThreadSafeQueue<Json::Value> m_incoming;
	ThreadSafeQueue<Json::Value> m_outgoing;

	//Used to find the correct handler to deal with messages (by type)
	sm4ns3::HandlerLookup handleLookup;

	//I/O needs to go in its own thread, which is spun off via this function.
	boost::asio::io_service io_service;
	ns3::SystemThread iorun_thread;
	void run_io_service();

public:
	//TODO: Surely we can avoid these.
	static unsigned int m_global_tick;
	static unsigned int global_pckt_cnt;

	virtual bool start(std::string application = "stk");
	virtual void pause();
	bool processInitMessages();
	void sendClientDone();
	void processIncoming();
	void sendOutgoing();
	bool parsePacket(const std::string &input);

};


}


