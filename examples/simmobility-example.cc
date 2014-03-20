//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <iostream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/sm4ns3-module.h"

using namespace ns3;


//Command line parameters.
std::string application = "stk";
std::string host = "localhost";
std::string port = "6745";


void parse_command_line(int argc, char* argv[]) 
{
	//General help line.
	CommandLine cmd;
	cmd.Usage ("Run a client for Sim Mobility that will dispatch all network activity over ns-3.\n");

	//Add all parameters.
	cmd.AddValue ("application", "current running application", application);
	cmd.AddValue ("host", "Address of Sim Mobility server to connect to.", host);
	cmd.AddValue ("port", "Port of Sim Mobility server to connect to", port);

	//Parse all parameters
	cmd.Parse (argc, argv);
}


int main(int argc, char* argv[])
{
	parse_command_line(argc, argv);	
	std::cout << "sm4ns3 starterd; application: " <<application << "; host: " <<host <<"; port: " <<port <<"\n";
	
	// disable fragmentation for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));

	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));

	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue ("DsssRate1Mbps"));
	
	//Start up the broker.
	sm4ns3::Broker broker(host, port);
	if(!broker.start(application)) {
		std::cout <<"ERROR: Broker Start Failed";
		return 1;
	}

	//Run the Simulator, and cleanup when it's done.
	Simulator::Run();
	Simulator::Destroy();
	std::cout <<"sm4ns3 - Simulation Destroyed\n";
	return 0;
}


