//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_base_ip.h"
#include "ns3/log.h"

sm4ns3::BaseIP::BaseIP()
{
	m_last_ip_assignment.Set("0.0.0.1");
	m_ipv4.SetBase ("11.0.0.0", "255.0.0.0",m_last_ip_assignment );
}


void sm4ns3::BaseIP::configure()
{
}

void sm4ns3::BaseIP::SetBase(const ns3::Ipv4Address network, const ns3::Ipv4Mask mask, const ns3::Ipv4Address address)
{
	m_ipv4.SetBase (network, mask, address);
}

ns3::Ipv4InterfaceContainer sm4ns3::BaseIP::Install_ip(ns3::NetDeviceContainer &devices) 
{
	return m_ipv4.Assign(devices);
}

void sm4ns3::BaseIP::Install_inet(ns3::NodeContainer &nc) 
{
	m_internet.Install (nc);
}

ns3::Ipv4InterfaceContainer sm4ns3::BaseIP::Install(ns3::NodeContainer &nc, ns3::NetDeviceContainer &devices) 
{
	Install_inet (nc);
	return Install_ip(devices);
}

