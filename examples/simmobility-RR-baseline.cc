#include <iostream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/sm4ns3-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;
using std::string;

//Command line parameters.
string application = "Default";
string trace = "./src/simmobility/roadrunner_testing/trace_with_time.txt";
string out = "./src/simmobility/roadrunner_testing/out.txt";
std::string protocol = "80211b";
unsigned int nAgent = 10000;
bool disable_communication = false;
bool disable_location_update = false;


void parse_command_line(int argc, char* argv[]) 
{
	//General help line.
	CommandLine cmd;
	cmd.Usage ("Re-run a trace file of a previous Sim Mobility + ns3 simulation.\n");

	//Add all parameters
	cmd.AddValue("application", "current running application", application);
	cmd.AddValue("trace", "the trace file to read from", trace);
	cmd.AddValue("nAgent", "Number of agents in the simulation", nAgent);
	cmd.AddValue("protocol", "Protocol in use (80211p,80211b)", protocol);
	cmd.AddValue("output", "output file", out);
	cmd.AddValue("disable-communication", "Ignore Multicast lines", disable_communication);	
	cmd.AddValue("disable-location-update", "Ignore location(position) update lines", disable_location_update);

	//Parse all parameters
	cmd.Parse (argc, argv);
}


int main(int argc, char *argv[]) 
{
	parse_command_line(argc, argv);	
	std::cout << "sm4ns3-trace starterd; application: " <<application << "; nAgent: " <<nAgent <<"; disable: " <<(disable_communication?"comm,":"N/A,") <<(disable_location_update?"loc":"N/A") <<"\n";

	std::string phyMode("DsssRate1Mbps");

	// disable fragmentation for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",StringValue("2200"));

	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",StringValue("2200"));

	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue(phyMode));

	std::cout <<"Starting Broker\n";
	sm4ns3::RoadRunnerBaseLine broker(protocol, nAgent, out,disable_communication,disable_location_update);
	if (!broker.start(trace)) {
		std::cout <<"Broker Start Failed";
	}

	//Run to completion.
	Simulator::Run();
	Simulator::Destroy();
	std::cout <<"Simulation Destroyed, joining the ASIO thread now\n";

	return 0;
}

