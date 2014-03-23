//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once 

//NOTE: Do not move/remove this. NS-3 has a weird header bug that requires this to be included before boost::asio
#include "ns3/internet-module.h"

#include <string>
#include <vector>
#include <map>
#include <set>


#include "smb_broker.h"
#include "message_base.h"

#include "ns3/node-container.h"


namespace sm4ns3 {

//Forward Declaration
class ProfileBuilder;


//These structs cache the data found in the trace file, so that it can be pulled up faster.
struct TickMulticast {
	unsigned int sendAgId;
	std::vector<unsigned int> receiveAgId;
	std::string mcData;
};
struct TimeTick {
	std::vector<unsigned int> add_agent; //agentId
	std::map<unsigned int, DPoint> update_agent;  //agentID => (x,y)
	std::vector<TickMulticast> multicast;
};


class RoadRunnerBaseLine : public sm4ns3::Broker {
public:
	RoadRunnerBaseLine(unsigned int nof_agents, const std::string& outputFile, bool disable_communication, bool disable_location_update);
	~RoadRunnerBaseLine();

	void sendOutgoing();

	void pushToQueue();
	void reset();
	virtual bool start(std::string parameter = "");
	virtual void pause();

private:
	///Parse a trace file.
	void parse(const std::string& fileName);

	///Parse a line in the trace file.
	void parseLine(const std::string &line);

	//Create all Nodes that will be in the network.
	void createNodes();

	//Our cached data
	std::map<unsigned int, TimeTick> trace_time_ticks;

	//Helper: known agent ids (used for building only)
	std::set<unsigned int> agent_limit;

	//How we iterate.
	std::map<unsigned int, TimeTick>::const_iterator prevTick;
	std::map<unsigned int, TimeTick>::const_iterator currTick;

	//Timing!
	clock_t start_time;
	clock_t end_time;

	static ns3::NodeContainer RRNC;

	//Easy lookup of message types.
	enum messageType { MULTICAST, ADD, UPDATE };
	std::map<std::string, messageType> MessageMap;

	unsigned int interval;

	bool disable_communication;
	bool disable_location_update;

	unsigned int maxNOFAgents;

	//this is used when we want to invoke only a 
	//limited number of agents in an input file.
	//in order to search less,the agent ids are stored here in ascending order
	//then the id of nth agent in the set is obtained('max id').
	//and during the simulation, the lines having id 
	//greter than the 'max id' are discarded.
	std::set<unsigned int> agentsList;//temporary holder
	ProfileBuilder* profiler;

};

}


