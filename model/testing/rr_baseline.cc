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
#include "ns3/wave-mac-helper.h"
#include "ns3/wifi-80211p-helper.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "smb_agent.h"
#include "serialize.h"
#include "profile_builder.h"



NS_LOG_COMPONENT_DEFINE("RoadRunnerBaseLineComponent");

using namespace ns3;

ns3::NodeContainer sm4ns3::RoadRunnerBaseLine::RRNC;


sm4ns3::RoadRunnerBaseLine::RoadRunnerBaseLine(const std::string& protocol, unsigned int nof_agents, const std::string& outputFile, bool disable_communication, bool disable_location_update) :
	protocol(protocol), interval(0),
	disable_communication(disable_communication), disable_location_update(disable_location_update),
	maxNOFAgents(nof_agents)
{
	MessageMap["multicast"] = RoadRunnerBaseLine::MULTICAST;
	MessageMap["add_agent"] = RoadRunnerBaseLine::ADD;
	MessageMap["update_agent"] = RoadRunnerBaseLine::UPDATE;

	ProfileBuilder::InitLogFile(outputFile);
	profiler = new ProfileBuilder();
}

sm4ns3::RoadRunnerBaseLine::~RoadRunnerBaseLine()
{
	delete profiler;
}

void sm4ns3::RoadRunnerBaseLine::sendOutgoing()
{
	JsonParser::serialize_begin(m_outgoing);
}

//NOTE: The "add_agent" list is maintained, but it seems that this list is NOT used to generate "AddAgent" messages (these are instead done at initialization time, in parseLine()).
void sm4ns3::RoadRunnerBaseLine::pushToQueue()
{
	//Typical serialization
	OngoingSerialization ongoing;
	JsonParser::serialize_begin(ongoing);

	//Create a fake ALL_LOCATIONS_DATA message with agent location updates.
	if (!currTick->second.update_agent.empty()) {
		JsonParser::makeAllLocations(ongoing, currTick->second.update_agent);
	}

	//Create fake Multicast messages.
	for (std::vector<TickMulticast>::const_iterator mcIt=currTick->second.multicast.begin(); mcIt!=currTick->second.multicast.end(); mcIt++) {
		JsonParser::makeMulticast(ongoing, mcIt->sendAgId, mcIt->receiveAgId, mcIt->mcData);
	}

	//Done, serialize.
	BundleHeader mHead;
	std::string mStr;
	JsonParser::serialize_end(ongoing, mHead, mStr);

	//Now, immediately de-serialize.
	MessageConglomerate messages;
	JsonParser::deserialize(mHead, mStr, messages);

	//Push all messages to the Broker's queue.
	m_incoming.push(messages);
}


void sm4ns3::RoadRunnerBaseLine::createNodes()
{
	//Initialize our Nodes.
	RRNC.Create(maxNOFAgents);

	NetDeviceContainer rrDevices;
	if (protocol == "80211b") {
		//channel
		ns3::YansWifiChannelHelper wifiChannel;
		wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

		// The below FixedRssLossModel will cause the rss to be fixed regardless
		// of the distance between the two stations, and the transmit power
		wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", ns3::DoubleValue(-80));

		//phy
		YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
		wifiPhy.Set("RxGain", ns3::DoubleValue(0));
		wifiPhy.SetPcapDataLinkType(ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
		wifiPhy.SetChannel(wifiChannel.Create());

		//mac
		NqosWifiMacHelper mac = NqosWifiMacHelper::Default();
		mac.SetType("ns3::AdhocWifiMac");

		//wifi
		WifiHelper wifi;
		wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);
		wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("DsssRate1Mbps"), "ControlMode", ns3::StringValue("DsssRate1Mbps"));
		rrDevices = wifi.Install(wifiPhy, mac, RRNC);
	} else if(protocol == "80211p") {
		//channel
		ns3::YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();

		//required setting:
		wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", ns3::DoubleValue(-80));
		Ptr<YansWifiChannel> channel = wifiChannel.Create ();

		//phy
		YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
		wifiPhy.Set("RxGain", ns3::DoubleValue(0));
		wifiPhy.SetPcapDataLinkType(ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
		wifiPhy.SetChannel (channel);

		// ns-3 supports generate a pcap trace
		wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

		//mac
		NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();

		//wifi
		Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
		std::string phyMode ("OfdmRate6MbpsBW10MHz");
		wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));
		rrDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, RRNC);
	} else{
		throw std::runtime_error("Unknown Wifi Protocol Specified");
	}

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

		//our desirable information("agent_id") start from token[2] + x,y
		//our desirable information("agent_id") is in token[2]
		//x:token[3], y:token[4]
		std::string xStr = tokens[3].substr(tokens[3].find(":")+1 , tokens[3].size() - 1);
		std::string yStr = tokens[4].substr(tokens[4].find(":")+1 , tokens[4].size() - 1);
		double xPos = boost::lexical_cast<double>(xStr);
		double yPos = boost::lexical_cast<double>(yStr);

		trace_time_ticks[currTime].update_agent[agId] = DPoint(xPos, yPos);
		break;
	}

	case MULTICAST:{
		if(disable_communication){ break; }
		if (agent_limit.find(agId)==agent_limit.end()) { break; }
		TickMulticast res;
		res.sendAgId = agId;

		//Get the recepients from token[3]
		std::string recipientsStr = tokens[3].substr(tokens[3].find(":")+1 , tokens[3].size() - 1);
		std::vector<std::string> tempVec;
		boost::split(tempVec, recipientsStr, boost::is_any_of(","));

		for (std::vector<std::string>::const_iterator it=tempVec.begin(); it!=tempVec.end(); it++) {
			if (!it->empty()) {
				res.receiveAgId.push_back(boost::lexical_cast<unsigned int>(*it));
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

	//Push all messages for this time tick.
	pushToQueue();

	//Increment.
	prevTick = currTick;
	currTick++;

	//Set the interval. The very last time tick will just use the previous interval value, which is probably correct.
	if (currTick!=trace_time_ticks.end()) {
		interval = currTick->first - prevTick->first;
	}

	//Get the tick ID from the interval
	unsigned int tickId = prevTick->first/interval; //sort of an approximation

	//Now, react to all this data we just pushed.
	profiler->logTickBegin(tickId, prevTick->second.update_agent.size(), 0);
	Broker::processIncoming();
	profiler->logTickEnd(tickId, prevTick->second.update_agent.size(), 0);

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


