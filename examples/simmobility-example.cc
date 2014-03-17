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
#include "ns3/simmobility-module.h"

using namespace ns3;

//Define a log component for our example.
NS_LOG_COMPONENT_DEFINE ("SimMobilityExample");

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
	NS_LOG_DEBUG( "Application: " <<application << "; host: " <<host <<"; port: " <<port);
	
	// disable fragmentation for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));

	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));

	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue ("DsssRate1Mbps"));
	
	sim_mob::BaseFactory<sim_mob::Registration*> & appRegFactory = sim_mob::Registration::getFactory();
	sim_mob::BaseFactory<sim_mob::Agent*> & agentFactory = sim_mob::Agent::getFactory();

	NS_LOG_DEBUG( "declaring a broker");
	sim_mob::Broker broker(host, port);
	NS_LOG_DEBUG( "declaring a broker: done" );

	NS_LOG_DEBUG( "Register  Registration" );
	appRegFactory.Register("Default", new sim_mob::Registration(&broker,host, port));
	NS_LOG_DEBUG( "Register  Registration: Done" );

	NS_LOG_DEBUG( "Register  WFD_Registration" );
	appRegFactory.Register("stk", new sim_mob::WFD_Registration(&broker,host, port));
	NS_LOG_DEBUG( "Register  WFD_Registration: Done" );

	NS_LOG_DEBUG( "Register  Agent" );
	agentFactory.Register("Default", new sim_mob::Agent());
	NS_LOG_DEBUG( "Register  Agent Done" );

	NS_LOG_DEBUG( "Register  WFD_Agent" );
	agentFactory.Register("stk", new sim_mob::WFD_Agent());
	NS_LOG_DEBUG( "Register  WFD_Agent Done" );

	if(!broker.start(application)) {
		NS_LOG_ERROR( "Broker Start Failed" );
	}

	//Run the Simulator, and cleanup when it's done.
	Simulator::Run();
	Simulator::Destroy ();
	NS_LOG_DEBUG("Simulation Destroyed");
	return 0;
}


