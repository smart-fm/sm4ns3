//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "smb_connection.h"

#include <iostream>

#include <boost/asio/io_service.hpp>

#include <jsoncpp/json/json.h>

#include "ns3/system-mutex.h"
#include "ns3/system-thread.h"

#include "serialize.h"
#include "thread_safe_queue.h"
#include "handlers.h"



namespace sm4ns3 {



///Contains various callback functions, allowing other classes to interact with Brokers without requiring the entire object.
class BrokerBase {
public:
	///Used by Connection to send messagse back to the Broker. Called asynchronously; make sure to lock properly.
	virtual void onMessageReceived(const BundleHeader& header, const std::string& message) = 0;

	///Used by Registration and WFD_Registration
	virtual Connection& getConnection() = 0;

	//Used by Agent, WFD_Agent
	virtual OngoingSerialization& getOutgoing() = 0;
};


class Broker : public BrokerBase{
public:
	Broker(const std::string& simmob_host="localhost", const std::string& simmob_port="6745");
	virtual ~Broker();

	//Overriding BrokerBase
	virtual void onMessageReceived(const BundleHeader& header, const std::string& message);
	virtual Connection& getConnection();
	virtual OngoingSerialization& getOutgoing();

private:
	//Connection to Sim Mobility.
	sm4ns3::Connection conn;

	//This variable+mutex is used to advance the program between reading from a network stream and performing simulation.
	ns3::SystemMutex mutex_pause;
	bool m_pause;

	//Used to find the correct handler to deal with messages (by type)
	sm4ns3::HandlerLookup handleLookup;

	//I/O needs to go in its own thread, which is spun off via this function.
	boost::asio::io_service io_service;
	ns3::SystemThread iorun_thread;
	void run_io_service();

protected:
	//Since both threads (network+main) interact with the incoming message buffer, we use a ThreadSafeQueue to safely access it.
	ThreadSafeQueue<MessageConglomerate> m_incoming;

	//Our current in-progress serialization of all outgoing messages.
	//NOTE: I am *mostly* convinced that this is only updated by one thread, since the only things that post messages are Agents (which are only updated by the ns3 main thread).
	OngoingSerialization m_outgoing;

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
	bool parsePacket(const BundleHeader& header, const std::string &input);

};


}


