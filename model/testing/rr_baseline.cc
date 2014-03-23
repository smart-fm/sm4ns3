//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "rr_baseline.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sys/time.h>

#include "ns3/internet-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "smb_agent.h"
#include "serialize.h"
#include "profile_builder.h"



NS_LOG_COMPONENT_DEFINE("RoadRunnerBaseLineComponent");

using namespace ns3;

ns3::NodeContainer sm4ns3::RoadRunnerBaseLine::RRNC;


sm4ns3::RoadRunnerBaseLine::RoadRunnerBaseLine(unsigned int nof_agents, const std::string& outputFile, bool disable_communication, bool disable_location_update) :
	currTime(0), nextTime(0), disable_communication(disable_communication), disable_location_update(disable_location_update),
	temp_nof_comm(0), maxNOFAgents(nof_agents)
{
	MessageMap["multicast"] = RoadRunnerBaseLine::MULTICAST;
	MessageMap["add_agent"] = RoadRunnerBaseLine::ADD;
	MessageMap["update_agent"] = RoadRunnerBaseLine::UPDATE;

	RealtimeInterval_in_mill = 0;
	ProfileBuilder::InitLogFile(outputFile);
	profiler = new ProfileBuilder();
}

sm4ns3::RoadRunnerBaseLine::~RoadRunnerBaseLine()
{
	try {
		std::cout <<"Destructing RoadRunnerBaseLine\n";
		//fileReader.join();
	} catch ( const boost::thread_interrupted& ) {
		std::cout <<"Warning: interrupted thread.\n"; //This might be common.
	}
	delete profiler;
}

void sm4ns3::RoadRunnerBaseLine::sendOutgoing()
{
	m_outgoing.clear();
}

void sm4ns3::RoadRunnerBaseLine::pushToQueue()
{
	//Create a fake AgentsInfo message with ADD agents.
	if(!agents_to_add.empty()) {
		std::vector<unsigned int> rem;
		Json::Value msg = JsonParser::makeAgentsInfo(agents_to_add, rem);

		//Push it to the broker's queue
		m_incoming.push(msg);
	}

	//Create a fake ALL_LOCATIONS_DATA message with agent location updates.
	if(!agentsLocation.empty()) {
		Json::Value msg = JsonParser::makeAllLocations(agentsLocation);

		//Push it to the broker's queue
		m_incoming.push(msg);
	}

	//Create fake Multicast messages.
	if(multicasts.size()){
		for (std::list<multicast_data>::const_iterator it=multicasts.begin(); it!=multicasts.end(); it++) {
			Json::Value msg = JsonParser::makeMulticast(it->sending_agent, it->recipient_agents, it->data);

			//Push it to the broker's queue
			m_incoming.push(msg);
		}
	}
}


void sm4ns3::RoadRunnerBaseLine::reset()
{
	agents_to_add.clear();
	agentsLocation.clear();
	multicasts.clear();
	currTime = nextTime;
}


void sm4ns3::RoadRunnerBaseLine::createNodes()
{
	//Initialize our Nodes.
	RRNC.Create(maxNOFAgents);

	//Set up WiFi
	WifiHelper wifi;
	wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);
	
	NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("DsssRate1Mbps"), "ControlMode",ns3::StringValue ("DsssRate1Mbps"));

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	wifiPhy.Set("RxGain", ns3::DoubleValue(0));
	wifiPhy.SetPcapDataLinkType(ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

	ns3::YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

	// The below FixedRssLossModel will cause the rss to be fixed regardless
	// of the distance between the two stations, and the transmit power
	wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", ns3::DoubleValue(-80));
	wifiPhy.SetChannel (wifiChannel.Create ());
	mac.SetType ("ns3::AdhocWifiMac");

	NetDeviceContainer rrDevices = wifi.Install (wifiPhy, mac, RRNC);

	//Internet: Add the IPv4 protocol stack to the nodes in our container
	InternetStackHelper internet;
	internet.Install(RRNC);

	//Assign IPv4 addresses to the device drivers (actually to the associated IPv4 interfaces) we just created.
	Ipv4AddressHelper ipAddrs;
	ipAddrs.SetBase ("10.0.0.0", "255.255.0.0");
	ns3::Ipv4InterfaceContainer iic = ipAddrs.Assign (rrDevices);

	//mobility
	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install(RRNC);
	
	std::set<unsigned int>::const_iterator it_agent = agentsList.begin();
	ns3::Ipv4InterfaceContainer::Iterator it_ii = iic.Begin();
	uint32_t i = 0;
	while(i != RRNC.GetN()){
		Ptr<Node> node = RRNC.Get(i);
		Ptr<Agent> agent = ns3::CreateObject<sm4ns3::Agent>(*it_agent, node, this);
		sm4ns3::Agent::AddAgent(*it_agent, agent);
		agent->iic.Add(*it_ii);
		agent->initSocket();

		i++;
		it_agent++;
		it_ii++;
	}

	std::cout <<"Size of Node Container " <<Agent::getAgents().size() <<"\n";

	//TODO: replace with Profiler line.
	prevRealTimeTime = currRealTime;
	gettimeofday(&currRealTime ,0);
	RealtimeInterval_in_mill =  ((currRealTime.tv_sec) * 1000 + (currRealTime.tv_usec) / 1000) - ((prevRealTimeTime.tv_sec) * 1000 + (prevRealTimeTime.tv_usec) / 1000) ; // convert tv_sec & tv_usec to millisecond//		
	NS_LOG_UNCOND("Agent Creation ended at : " << RealtimeInterval_in_mill << " ms");
}



bool sm4ns3::RoadRunnerBaseLine::start(std::string fileName)
{
	//First, parse the input file.
	parse(fileName);

	//Now generate all nodes.
	createNodes();

	//Prepare to start the simulation.
	prevTick = trace_time_ticks.end(); 
	currTick = trace_time_ticks.begin(); 
			
	//real time
	prevRealTimeTime = currRealTime;
	gettimeofday(&currRealTime ,0);
	RealtimeInterval_in_mill =  ((currRealTime.tv_sec) * 1000 + (currRealTime.tv_usec) / 1000) - ((prevRealTimeTime.tv_sec) * 1000 + (prevRealTimeTime.tv_usec) / 1000) ; // convert tv_sec & tv_usec to millisecond

	//Profiled time starts here.
	profiler->logSiMobilityBegin();

	//We output the most likely useful information (total runtime).
	start_time = clock();
	Simulator::ScheduleNow(&sm4ns3::RoadRunnerBaseLine::pause, this);
	return true;
}


void sm4ns3::RoadRunnerBaseLine::parse(const std::string& fileName) 
{
	//We are actually going to parse the entire file all at once. This will take a while.
	trace_time_ticks.clear();
	sleep(2); //TODO: Why?

	//Open it.
	std::ifstream infile;
	infile.open(fileName.c_str(), std::ifstream::in);
	if (!infile.is_open()) { throw std::runtime_error("Input file not found."); }

	//Read line-by-line.
	std::string line;
	unsigned int lineNum = 0;
	while (std::getline(infile, line)) {
		//Parse
		parseLine(line);
		lineNum++;

		//Some feedback.
		if(lineNum%20000000==0) {
			std::cout <<"Parsed line: " <<lineNum <<std::endl;
		}
	}

	//Some feedback.
	std::cout <<"Total agents: " <<agent_limit.size() <<(agent_limit.size()<maxNOFAgents?" (TOO LOW)":" (expected count)") <<"\n";
}

void sm4ns3::RoadRunnerBaseLine::parseLine(const std::string &line)
{
	//Split into tokens.
	std::vector<std::string> tokens;
	boost::split(tokens, line, boost::is_any_of(";"));

	//The time tick notification always comes first.
	if (tokens.size() == 1) {
		unsigned int nextTime = boost::lexical_cast<unsigned int>(tokens[0]);
		trace_time_ticks[nextTime] = TimeTick();
		return;
	}

	//message type (token[0])
	const std::string& messageTypeStr = tokens[0];

	//time (token[1])
	//"time:10ms" => .substr( (8 - 5) - (8-6) )
	std::string timeStr = tokens[1].substr(5,(tokens[1].size() - 5) - (tokens[1].size() - tokens[1].find("ms")));
	unsigned int currTime = boost::lexical_cast<unsigned int>(timeStr); //NOTE: May need +1 here.

	//agent (token[2])
	std::string agIdStr = tokens[2].substr(tokens[2].find(":")+1 , tokens[2].size() - tokens[2].find(":"));
	unsigned int agId = boost::lexical_cast<unsigned int>(agIdStr);


	//based on the line-type token, process the rest of the tokens
	switch(MessageMap[messageTypeStr]){
	case ADD:{
		if (agent_limit.size()<maxNOFAgents) {
			agent_limit.insert(agId);
		}

		//Add it.
		if (agent_limit.find(agId)==agent_limit.end()) { break; }
		trace_time_ticks[currTime].add_agent.push_back(agId);

		//Again
		agentsList.insert(agId);
		break; 
	}
	case UPDATE: {
		if(disable_location_update) { break; }
		if (agent_limit.find(agId)==agent_limit.end()) { break; }
		TickUpdate res;
		res.agId = agId;

		//our desirable information("agent_id") start from token[2] + x,y
		//our desirable information("agent_id") is in token[2]
		//x:token[3], y:token[4]
		std::string xStr = tokens[3].substr(tokens[3].find(":")+1 , tokens[3].size() - 1);
		std::string yStr = tokens[4].substr(tokens[4].find(":")+1 , tokens[4].size() - 1);
		res.xPos = boost::lexical_cast<unsigned int>(xStr);
		res.yPos = boost::lexical_cast<unsigned int>(yStr);

		trace_time_ticks[currTime].update_agent.push_back(res);
		break;
	}

	case MULTICAST:{
		if(disable_communication){ break; }
		if (agent_limit.find(agId)==agent_limit.end()) { break; }
		TickMulticast res;
		res.agId = agId;

		//Get the recepients from token[3]
		std::string recipientsStr = tokens[3].substr(tokens[3].find(":")+1 , tokens[3].size() - 1);
		std::vector<std::string> tempVec;
		boost::split(tempVec, recipientsStr, boost::is_any_of(","));

		for (std::vector<std::string>::const_iterator it=tempVec.begin(); it!=tempVec.end(); it++) {
			if (!it->empty()) {
				res.agentIds.push_back(boost::lexical_cast<unsigned int>(*it));
			}
		}

		//Retrieve the data from token[4]
		res.mcData = tokens[4];

		trace_time_ticks[currTime].multicast.push_back(res);
		break;
	}
	default:
		throw std::runtime_error("Unknown message type.");
	}
}



void sm4ns3::RoadRunnerBaseLine::pause() 
{
	//Simple feedback.
	unsigned int absTick = currTick->first; //An approximation.
	if(absTick>0 && absTick%(trace_time_ticks.size()/10) == 0) {
		std::cout <<"Time ticks done: " <<(absTick*100)/trace_time_ticks.size() <<"%" << std::endl;
	}

	//Process all updates/messages FIRST.
	for (std::vector<TickUpdate>::const_iterator upIt=currTick->second.update_agent.begin(); upIt!=currTick->second.update_agent.end(); upIt++) {
		if(agentsList.find(upIt->agId) == agentsList.end()){ continue; }

		agentsLocation[upIt->agId] = DPoint(upIt->xPos, upIt->yPos);
	}
	for (std::vector<TickMulticast>::const_iterator mcIt=currTick->second.multicast.begin(); mcIt!=currTick->second.multicast.end(); mcIt++) {
		if(agentsList.find(mcIt->agId) == agentsList.end()){ continue; }

		std::vector<unsigned int> agentIds;
		for (std::vector<unsigned int>::const_iterator recvIt=mcIt->agentIds.begin(); recvIt!=mcIt->agentIds.end(); recvIt++) {
			if(agentsList.find(*recvIt) == agentsList.end()){
				continue;
			}
			agentIds.push_back(*recvIt);
		}
		multicasts.insert(multicasts.begin(), multicast_data(mcIt->agId,agentIds,mcIt->mcData));
	}

	//Set the interval (kind of hackish)
	if (prevTick==trace_time_ticks.end()) {
		std::map<unsigned int, TimeTick>::const_iterator nextTick = currTick;
		interval = ((++nextTick)->first) - currTick->first; //Estimate
	} else {
		interval = currTick->first - prevTick->first;
	}

	//Increment.
	prevTick = currTick;
	currTick++;

	//Finally, announce the time tick.
	pushToQueue();

	//Now, react to all this data we just pushed.
	profiler->logTickBegin(currTick->first/interval, agentsLocation.size(),temp_nof_comm);
	Broker::processIncoming();
	profiler->logTickEnd(currTick->first/interval, agentsLocation.size(),temp_nof_comm);

	//some resettings
	temp_nof_comm = 0;
	reset();//changing its place in order to get their statistics first(agent size)

	//plan for the next tick
	if (currTick != trace_time_ticks.end()) {
		Simulator::Schedule(MilliSeconds(interval), &RoadRunnerBaseLine::pause, this);
	} else {
		//Output runtime; if all you need is total runtime, then just use this.
		end_time = clock();
		double seconds = ((double)(end_time-start_time))/CLOCKS_PER_SEC;
		std::cout <<"Total runtime: " <<seconds <<" s\n";

		std::cout <<"Stopping the Simulation\n";
		profiler->logSiMobilityEnd();
		Simulator::Stop();
	}
}


