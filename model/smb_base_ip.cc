/*
 * smb_base_ip.cc
 *
 *  Created on: Aug 6, 2013
 *      Author: vahid
 */

#include "smb_base_ip.h"
#include "ns3/log.h"

//#include "ns3/core-module.h"
namespace sim_mob {

BaseIP::BaseIP()/*:m_last_ip_assignment("10.0.0.1")*/ {
	init();
}

void BaseIP::init() {
	m_last_ip_assignment.Set("0.0.0.1");
	m_ipv4.SetBase ("11.0.0.0", "255.0.0.0",m_last_ip_assignment );
}
void BaseIP::configure(){

}

void BaseIP::SetBase(
const ns3::Ipv4Address network,
const ns3::Ipv4Mask mask,
const ns3::Ipv4Address address)
{
	m_ipv4.SetBase (network, mask,address);
}
//
//
//void BaseIP::SetBase(
//const ns3::Ipv4Address network,
//const ns3::Ipv4Mask mask)
//{
//	m_ipv4.SetBase (network, mask);
//}

ns3::Ipv4InterfaceContainer BaseIP::Install_ip(ns3::NetDeviceContainer &devices) {
	ns3::Ipv4InterfaceContainer i = m_ipv4.Assign(devices);
//	NS_LOG_UNCOND( i.GetAddress(0));
	return i;
}

void BaseIP::Install_inet(ns3::NodeContainer &nc) {
//	ns3::InternetStackHelper m_internet;
	m_internet.Install (nc);
}

ns3::Ipv4InterfaceContainer  BaseIP::Install(ns3::NodeContainer &nc, ns3::NetDeviceContainer &devices) {
	Install_inet (nc);
	return Install_ip(devices);
}

BaseIP::~BaseIP() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
