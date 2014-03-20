//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "ns3/ipv4-address-helper.h"
#include "ns3/internet-stack-helper.h"

namespace ns3 {
class NodeContainer;
} //End namespace ns3

namespace sm4ns3 {

class BaseIP {
public:
	BaseIP();

	virtual void configure();

	void SetBase(const ns3::Ipv4Address network, const ns3::Ipv4Mask mask, const ns3::Ipv4Address address);

	ns3::Ipv4InterfaceContainer Install_ip(ns3::NetDeviceContainer &devices);

	void Install_inet(ns3::NodeContainer &nc);

	ns3::Ipv4InterfaceContainer Install(ns3::NodeContainer &nc, ns3::NetDeviceContainer &devices);

private:
	ns3::Ipv4AddressHelper m_ipv4;
	ns3::Ipv4Address m_last_ip_assignment;
	ns3::InternetStackHelper m_internet;
};

}

