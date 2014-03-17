/*
 * smb_base_ip.h
 *
 *  Created on: Aug 6, 2013
 *      Author: vahid
 */

#pragma once

//#include "ns3/internet-module.h"

#include "ns3/ipv4-address-helper.h"
#include "ns3/internet-stack-helper.h"
#include "smb_configurator.h"

namespace ns3 {
class NodeContainer;
}

namespace sim_mob {

class BaseIP : public Configurator{
public:
	ns3::Ipv4AddressHelper m_ipv4;
	ns3::Ipv4Address m_last_ip_assignment;
	ns3::InternetStackHelper m_internet;
public:
	BaseIP();

	void init();
	virtual void configure();

	void SetBase(
			const ns3::Ipv4Address network,
			const ns3::Ipv4Mask mask,
			const ns3::Ipv4Address address);
//
//	void SetBase(
//			const ns3::Ipv4Address network,
//			const ns3::Ipv4Mask mask);


	ns3::Ipv4InterfaceContainer Install_ip(ns3::NetDeviceContainer &devices);

	void Install_inet(ns3::NodeContainer &nc);

	ns3::Ipv4InterfaceContainer
	Install(ns3::NodeContainer &nc,
			ns3::NetDeviceContainer &devices);

	virtual ~BaseIP();
};

} /* namespace sim_mob */
