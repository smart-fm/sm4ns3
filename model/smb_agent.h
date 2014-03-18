//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/log.h"

#include <vector>
#include <boost/unordered_map.hpp>
#include<boost/shared_ptr.hpp>
#include "ns3/vector.h"

#include "smb_base_wifi.h"
#include "smb_base_ip.h"
#include "smb_base_mobility.h"
#include "smb_message_factory_base.h"

namespace ns3 {
class Ipv4InterfaceContainer;
class WifiHelper;
class YansWifiPhyHelper;
class NqosWifiMacHelper;
}

using namespace ns3;
namespace sim_mob {
class Broker;
//NS_LOG_COMPONENT_DEFINE ("A NAME_sadjcbsajkl");
struct AgentConfig {
	unsigned int id;
	unsigned int type;
	Broker * broker;
	AgentConfig(unsigned int id,unsigned int type, Broker*broker):
		id(id),
		type(type),
		broker(broker){}
};
class Agent : public ns3::Object{
public:
	ns3::Ptr<ns3::Node> m_node; // Agent has a node
    int m_AgentId; // Agent's id
    sim_mob::Broker* m_parent_broker;
    ns3::Ptr<ns3::NetDevice> m_device; // Agent has a device

//    sim_mob::BaseWifi m_basic_wifi;
    ns3::NodeContainer nc;
    ns3::NetDeviceContainer ndc ;
    ns3::Ipv4InterfaceContainer iic;

    static BaseWifi m_wifi;
    static BaseMobility m_mobility;
	static BaseIP m_ip;

	ns3::Ptr<ns3::Socket> m_socket;
	ns3::InetSocketAddress *m_isa;


    //	all agents
    static boost::unordered_map<unsigned int,ns3::Ptr<Agent> > m_all_agents; //<m_AgentId, Agent*>

    /// agent factory(the best place to keep this factory is here itself(or may be a singleton)
//    static sim_mob::BaseFactory<Agent*> m_agentFactory;
public:
    /// Override TypeId.
    static ns3::TypeId GetTypeId(void);
    //	constructor
	Agent(int m_AgentId_, sim_mob::Broker* broker_);
	Agent(AgentConfig config);
	Agent();
	void DevRxTrace(std::string context, Ptr<const Packet> p);
	virtual void init();
	virtual ns3::Ptr<Agent> clone(int m_AgentId_, sim_mob::Broker* broker_);
//	static sim_mob::BaseFactory<Agent*> &getFactory();
	static void SetBasicNetworking(
			const ns3::WifiHelper &wifi = ns3::WifiHelper::Default(),
			const ns3::YansWifiPhyHelper &phy = ns3::YansWifiPhyHelper::Default(),
			const ns3::NqosWifiMacHelper &mac = ns3::NqosWifiMacHelper::Default());
//	void Configure(sim_mob::BaseWifi &baseWifi,
//			sim_mob::BaseMobility &baseMobility,
//			sim_mob::BaseIP &baseIP = m_ip);

//	void Configure(sim_mob::APP_TYPE);
////	//returns agent's InetSocketAddress
////	ns3::InetSocketAddress &getISA();
//	ns3::Ptr<ns3::Socket> & Socket();
	virtual bool configure();
	int Bind(uint16_t port = 80);
	void ReceivePacket (ns3::Ptr<ns3::Socket> socket);
//	//	destructor
	virtual ~Agent();
    static boost::unordered_map<unsigned int,ns3::Ptr<Agent> > &getAgents();
    static ns3::Ptr<Agent>& getAgent(unsigned int);
	//	add agents
	static void AddAgent(unsigned int , ns3::Ptr<Agent>);
	static void RemoveAgent(unsigned int);

//	/**
//     * \param wifi a WifiHelper.
//     * \param phy  a YansWifiPhyHelper.
//     * \param mac  a NqosWifiMacHelper.
//     *
//     * Setups the Agent wifi given the appropriate helpers.
//     * This function may need to be modified later if stable WAVE/DSRC standards added to ns-3.
//     */
//	virtual void SetupWifi(const ns3::WifiHelper &wifi, const ns3::YansWifiPhyHelper &phy, const ns3::NqosWifiMacHelper &mac);

    /**
     * \returns the Agent Id.
     *
     */
    int GetAgentId();
    /**
     * \param value a Agent Id.
     *
     * A Agent can have an Id. It is good that this Id be unique in the VANETs Highway.
     */
    void SetAgentId(int value);
    /**
     * \returns the position of Agent's Node which is loacted at the center back of the Agent.
     */
    ns3::Ptr<ns3::Node> GetNode();
    ns3::Vector GetPosition();
    /**
     * \param value a position Vector.
     *
     * This function sets the position of Agent's Node. Agent's position is its Node's position.
     * This position Vector must point to the center back of the Agent.
     */
    void SetPosition(ns3::Vector value);
    /**
     * \return the Wifi address of the Agent.
     *
     * Agents can use this address to communicate to each other in an unicast fashion.
     */
    ns3::Ipv4Address GetAddress();
    /**
     * \returns the Wifi broadcast address.
     *
     * A Agent can broadcast messages using this address.
     */
    ns3::Address GetBroadcastAddress();

//    sim_mob::BaseWifi & getBaseWifi();
    /**
     * \param agent the destination agent.
     * \param packet the packet to send.
     * \returns whatever the socket->Send returns
     */
    int SendTo(ns3::Ptr<Agent> agent, ns3::Ptr<ns3::Packet> packet);
};

/*
 * ******************************************************
 */

class WFD_Agent :public Agent{
public:
	enum AgentRole{
		WFD_GO,
		WFD_CLIENT
	};
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
    static InternetStackHelper wfd_internet;
    static ns3::MobilityHelper wfd_mobility;
    static ns3::Ptr<ListPositionAllocator> wfd_positionAlloc;
    static ns3::Ipv4AddressHelper wfd_ipAddrs;
public:

	WFD_Agent(int m_AgentId_, sim_mob::Broker* broker_);
	WFD_Agent();
	void init();
	AgentRole getRole();
	void setRole(AgentRole);
	ns3::Ptr<Agent> clone(int m_AgentId_, sim_mob::Broker* broker_);

	//which group are you in and which role do you have?
	void figureOutGroup();
	    /**
	     * \param agent the destination agent.
	     * \param packet the packet to send.
	     * \returns whatever the socket->Send returns
	     */
	static void configAll();
};

} /* namespace sim_mob */
