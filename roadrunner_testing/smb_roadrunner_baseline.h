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

#include "ns3/node-container.h"


namespace sm4ns3 {

//Forward Declaration
class ProfileBuilder;


//These structs cache the data found in the trace file, so that it can be pulled up faster.
struct TickUpdate {
	unsigned int agId;
	unsigned int xPos;
	unsigned int yPos;
};
struct TickMulticast {
	unsigned int agId;
	std::vector<unsigned int> agentIds;
	std::string mcData;
};
struct TimeTick {
	std::vector<unsigned int> add_agent; //agentId
	std::vector<TickUpdate> update_agent;
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
	enum messageType{
		MULTICAST,
		ADD,
		UPDATE
	};
	std::map<std::string, messageType> MessageMap;

	unsigned int currTime;
	unsigned int nextTime;
	unsigned int interval;

	bool disable_communication;
	bool disable_location_update;


	timeval currRealTime;
	timeval prevRealTimeTime;
	timeval RealTimeInterval;
	double RealtimeInterval_in_mill;


	//agents to add in each tick
	std::vector<unsigned int> agents_to_add;

	//agents location
	struct AgentLocation {
		unsigned int agent;
		unsigned int x;
		unsigned int y;
		AgentLocation() {}
		AgentLocation(unsigned int agent, unsigned int x, unsigned int y) : agent(agent),x(x),y(y){}
		bool operator<( const AgentLocation& rhs) const {
			return (agent < rhs.agent);
		}

	};

	//AgentId => x,y
	std::map<unsigned int, DPoint> agentsLocation;

	struct multicast_data {
		unsigned int sending_agent;
		std::vector<unsigned int> recipient_agents;
		std::string data;
		multicast_data() {}
		multicast_data(unsigned int sending_agent, std::vector<unsigned int> recipient_agents, std::string data) : 
			sending_agent(sending_agent), recipient_agents(recipient_agents), data(data) {}
	};

	std::list<multicast_data> multicasts;
	int temp_nof_comm;

	unsigned int maxNOFAgents;
	unsigned int maxAgentId;//temporary holder	

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


