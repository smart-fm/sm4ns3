//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/log.h"

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "ns3/vector.h"

#include "smb_base_wifi.h"
#include "smb_base_ip.h"
#include "smb_base_mobility.h"

namespace ns3 {
class Ipv4InterfaceContainer;
class WifiHelper;
class YansWifiPhyHelper;
class NqosWifiMacHelper;
} //End namespace ns3


namespace sm4ns3 {

class BrokerBase;


class Agent : public ns3::Object {
public:
	//	constructor
	Agent(int m_AgentId_, BrokerBase* broker);
	Agent();
	virtual ~Agent();

	/// Override TypeId.
	static ns3::TypeId GetTypeId(void);

	void DevRxTrace(std::string context, ns3::Ptr<const ns3::Packet> p);
	virtual void init();

	static void SetBasicNetworking(
		const ns3::WifiHelper &wifi = ns3::WifiHelper::Default(),
		const ns3::YansWifiPhyHelper &phy = ns3::YansWifiPhyHelper::Default(),
		const ns3::NqosWifiMacHelper &mac = ns3::NqosWifiMacHelper::Default()
	);

	virtual bool configure();
	int Bind(uint16_t port = 80);
	void ReceivePacket (ns3::Ptr<ns3::Socket> socket);

	static std::map<unsigned int,ns3::Ptr<Agent> > &getAgents();
	static ns3::Ptr<Agent>& getAgent(unsigned int);

	//	add agents
	static void AddAgent(unsigned int , ns3::Ptr<Agent>);
	static void RemoveAgent(unsigned int);

	///returns the Agent Id.
	int GetAgentId();

	///Agent can have an Id. It is good that this Id be unique in the VANETs Highway.
	void SetAgentId(int value);

	///\returns the position of Agent's Node which is loacted at the center back of the Agent.
	ns3::Ptr<ns3::Node> GetNode();

	ns3::Vector GetPosition();

	///This function sets the position of Agent's Node. Agent's position is its Node's position.
	///This position Vector must point to the center back of the Agent.
	void SetPosition(ns3::Vector value);

	///Agents can use this address to communicate to each other in an unicast fashion.
	///\return the Wifi address of the Agent.
	ns3::Ipv4Address GetAddress();

	///A Agent can broadcast messages using this address.
	///\returns the Wifi broadcast address.
	ns3::Address GetBroadcastAddress();


	///\param agent the destination agent.
	///\param packet the packet to send.
	///\returns whatever the socket->Send returns
	int SendTo(ns3::Ptr<Agent> agent, ns3::Ptr<ns3::Packet> packet);

public:
	//All agents, sorted by agent ID
	static std::map<unsigned int,ns3::Ptr<Agent> > AllAgents;

protected:
	//Interface to the Broker
	sm4ns3::BrokerBase* broker;

	int m_AgentId; // Agent's id
	ns3::Ptr<ns3::Node> m_node; // Agent has a node
	ns3::Ptr<ns3::NetDevice> m_device; // Agent has a device

	ns3::NodeContainer nc;
	ns3::NetDeviceContainer ndc ;
	ns3::Ipv4InterfaceContainer iic;

	static BaseWifi m_wifi;
	static BaseMobility m_mobility;
	static BaseIP m_ip;

	ns3::Ptr<ns3::Socket> m_socket;
	ns3::InetSocketAddress* m_isa;
};




class WFD_Agent :public Agent{
public:
	WFD_Agent(int m_AgentId_, BrokerBase* broker);
	WFD_Agent();
	virtual ~WFD_Agent();

	enum AgentRole{
		WFD_GO,
		WFD_CLIENT
	};

public:
	void init();
	AgentRole getRole();
	void setRole(AgentRole);

	//which group are you in and which role do you have?
	void figureOutGroup();

	///\param agent the destination agent.
	///\param packet the packet to send.
	///\returns whatever the socket->Send returns
	static void configAll();

private:
	ns3::Ptr<ns3::Node> m_wfd_node; // Agent has a node
	AgentRole role;
	unsigned int groupId;
	//todo, based n the experiences gained,
	//we can have a set of static members which will take care of
	//the overall initialization and setting,
	//if the results are good, remove wfd prefix

	//and apply this to the base class
	static ns3::NodeContainer wfd_nc;
	static ns3::NetDeviceContainer wfd_ndc ;
	static std::map<unsigned int , ns3::NodeContainer> wfd_go_nc;
	static std::map<unsigned int , ns3::NetDeviceContainer> wfd_go_ndc ;
	static std::map<unsigned int , ns3::NodeContainer> wfd_cln_nc;
	static std::map<unsigned int , ns3::NetDeviceContainer> wfd_cln_ndc ;
	static ns3::WifiHelper wfd_wifi;
	static ns3::NqosWifiMacHelper wfd_mac;
	static ns3::YansWifiPhyHelper wfd_wifiPhy;
	static ns3::YansWifiChannelHelper wfd_wifiChannel;
	static ns3::InternetStackHelper wfd_internet;
	static ns3::MobilityHelper wfd_mobility;
	static ns3::Ptr<ns3::ListPositionAllocator> wfd_positionAlloc;
	static ns3::Ipv4AddressHelper wfd_ipAddrs;
};

}


