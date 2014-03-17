//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_agent.h"
#include "smb_broker.h"
#include "smb_serializer.h"
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/olsr-helper.h"

namespace sim_mob {
NS_LOG_COMPONENT_DEFINE ("SimmobilityAgentLogComponent");
BaseWifi temp;
//todo: for now and for the reason I dont know yet, I need to initialize like this otherwise m_wifi will not be initialized properly
BaseWifi Agent::m_wifi = temp;
BaseMobility Agent::m_mobility;
BaseIP Agent::m_ip;
boost::unordered_map<unsigned int, ns3::Ptr<Agent> > Agent::m_all_agents;
sim_mob::BaseFactory<Agent*> Agent::m_agentFactory;

ns3::Ipv4Address
GetPacketSource(Ptr<Packet> packet)
{
	ns3::Ipv4Address src_ip;
  //The following code is based from
  //http://polythinking.wordpress.com/2012/05/30/ns-3-network-simulator-how-to-find-a-specific-header-in-packet-in-ns-3/

  // To get a header from Ptr<Packet> packet first, copy the packet
	Ptr<Packet> q = packet->Copy();
  // Use indicator to search the packet
  PacketMetadata::ItemIterator metadataIterator = q->BeginItem();
  PacketMetadata::Item item;
  while (metadataIterator.HasNext())
  {
    item = metadataIterator.Next();
    NS_LOG_UNCOND("item name: " << item.tid.GetName());

    // If we want to have an ip header
    if(item.tid.GetName() == "ns3::Ipv4Header")
    {
      Callback<ObjectBase *> constr = item.tid.GetConstructor();
      NS_ASSERT(!constr.IsNull());

      // Ptr<> and DynamicCast<> won't work here as all headers are from ObjectBase, not Object
      ObjectBase *instance = constr();
      NS_ASSERT(instance != 0);

      Ipv4Header* ipv4Header = dynamic_cast<Ipv4Header*> (instance);
      NS_ASSERT(ipv4Header != 0);

      ipv4Header->Deserialize(item.current);

      // The ipv4Header can now obtain the source of the packet
      src_ip = ipv4Header->GetSource();

      // Finished, clear the ip header and go back
      delete ipv4Header;
      break;
    }
  }
  return src_ip;
}

/*************************************************************************************
 * ************************************************************************************
 * ************************************************************************************
 * ************************************************************************************
 */
ns3::TypeId Agent::GetTypeId(void) {
    static ns3::TypeId tid = ns3::TypeId("sim_mob::Agent")
            .SetParent<ns3::Object > ()
            .AddConstructor<Agent > ()
            ;
    return tid;
}

boost::unordered_map<unsigned int,ns3::Ptr<Agent> > & Agent::getAgents(){
	return Agent::m_all_agents;
}

ns3::Ptr<Agent>& Agent::getAgent(unsigned int id){
	if(Agent::m_all_agents.find(id) == Agent::m_all_agents.end())
	{
		NS_LOG_UNCOND( "Agent::getAgent Agent not found[" << id  << "]" );
	}
	return Agent::m_all_agents[id];
}

Agent::Agent(int m_AgentId_, sim_mob::Broker* broker_):m_AgentId(m_AgentId_), m_parent_broker(broker_) {
	NS_LOG_UNCOND("Inside Agent::Agent(2)");
	Agent();
}
Agent::Agent(AgentConfig config):m_AgentId(config.id), m_parent_broker(config.broker) {
	NS_LOG_UNCOND("Inside Agent::Agent(1)");
	Agent();
}

Agent::Agent(){
//	NS_LOG_UNCOND("Inside Agent::Agent(0)" << this);
	m_isa = 0;
}


void Agent::DevRxTrace(std::string context, Ptr<const Packet> p) {
	std::cout << "Inside Agent::DevRxTrace" << " " << m_AgentId << std::endl;
//    if (!m_devRxTrace.IsNull())
//        m_devRxTrace(Ptr<Vehicle > (this), context, p);
}

void Agent::init()
{
	NS_LOG_UNCOND("Inside Agent::init");
	m_isa = 0;
	nc.Create(1);
	ndc = m_wifi.Install(nc);
	m_mobility.Install(nc);
	iic = m_ip.Install(nc, ndc);
	NS_LOG_UNCOND(iic.GetAddress(0));
	m_node = nc.Get(0);
	ns3::TypeId tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
	m_socket = ns3::Socket::CreateSocket(m_node, tid);
	m_socket->SetRecvCallback(MakeCallback(&Agent::ReceivePacket, this));
	Bind();
//	std::ostringstream oss;
//    oss << "/NodeList/" << m_node->GetId() << "/DeviceList/0/Mac/MacRx";
//    Config::Connect(oss.str(), MakeCallback(&Agent::DevRxTrace, this));
}

ns3::Ptr<Agent> Agent::clone(int m_AgentId_ , sim_mob::Broker* broker_){
	NS_LOG_UNCOND("Inside Agent::clone" << this);
	return ns3::CreateObject<Agent>(m_AgentId_,broker_);
}
//ns3::Ptr<Agent> Agent::clone(int m_AgentId_ id, sim_mob::Broker* broker_){
//	return ns3::CreateObject<Agent>(id,broker_);
//}
sim_mob::BaseFactory<Agent*> &Agent::getFactory(){
	return m_agentFactory;
}

bool Agent::configure(){
	return true;
}

int Agent::Bind(uint16_t port)
{
	if(m_isa)
	{
		delete m_isa;
		m_isa = 0;
	}
    m_isa = new ns3::InetSocketAddress (/*ns3::Ipv4Address::GetAny ()*/GetAddress(), port);
    if(!m_socket)
    {
    	return -1;
    }
    return m_socket->Bind(*m_isa);
}

void Agent::ReceivePacket (ns3::Ptr<ns3::Socket> socket)
{
//  ns3::Address addr;
//  ns3::Address remote_address;
//  socket->GetSockName (addr);
//  ns3::InetSocketAddress iaddr = ns3::InetSocketAddress::ConvertFrom (addr);
//  NS_LOG_UNCOND ("Agent " << m_AgentId <<  " Received data from Socket: " << iaddr.GetIpv4 () << " port: " << iaddr.GetPort ());
  ns3::Ptr<ns3::Packet> packet;
  std::string str;
  int i = 0;
  while ((packet = socket->Recv()))
    {
//	  ns3::InetSocketAddress remote_iaddr = ns3::InetSocketAddress::ConvertFrom (addr);
//	  NS_LOG_UNCOND ("Received data from remote Address : " << GetPacketSource(packet));
	  int size = packet->GetSize ();
      if (size > 0)
        {
    	  uint8_t buffer[size];
    	  packet->CopyData(buffer , size);
    	  str.assign(buffer , buffer+size);
          NS_LOG_UNCOND (i++ << ": Agent[" << m_AgentId << "] Received '"<< str << "'");
          Json::Value j_msg;
          JsonParser::parseMessage(str,j_msg);
          //change the sender and sender type information, the rest seems better to remain untouched :)
          j_msg["SENDER"] = "0";
          j_msg["SENDER_TYPE"] = "NS3_SIMULATOR";
//          boost::shared_ptr<sim_mob::comm::Message<Json::Value> >msg(new sim_mob::comm::Message<Json::Value>(j_msg));
          m_parent_broker->insertOutgoing(j_msg);
        }
    }
}

/*
 * Empty destructor
 */
Agent::~Agent() {
	if(m_isa)
	{
		delete m_isa;
		m_isa = 0;
	}
}

void Agent::AddAgent(unsigned int id, ns3::Ptr<Agent> value)
{
	m_all_agents.insert(std::make_pair(id, value));
//	NS_LOG_UNCOND( "inserting the agent into the map done--" << m_all_agents.size() );
}

void Agent::RemoveAgent(unsigned int id)
{
	boost::unordered_map<unsigned int, ns3::Ptr<Agent> >::iterator it;
	if((it = m_all_agents.find(id)) != m_all_agents.end())
	{
		it->second->Dispose();
		m_all_agents.erase(it);
	}
}

///*
// * Sets up Wifi for this vehicle using the given helpers
// */
//void Agent::SetupWifi(const ns3::WifiHelper &wifi, const ns3::YansWifiPhyHelper &phy, const ns3::NqosWifiMacHelper &mac) {
//
//}

/**
 * Returns the unique agent ID for this agent
 */
int Agent::GetAgentId() {
    return m_AgentId;
}

/**
 * Sets the agent ID (should be Unique, use IDGenerator)
 */
void Agent::SetAgentId(int value) {
	m_AgentId = value;
}

ns3::Ptr<ns3::Node> Agent::GetNode() {
	return m_node;
}

/**
 * Returns a ns3::Vector representing the location of the center of the
 * back bumper of this vehicle
 */
ns3::Vector Agent::GetPosition() {
    return m_node->GetObject<ns3::MobilityModel > ()->GetPosition();
}

/**
 * Sets the position of this vehicle
 */
void Agent::SetPosition(ns3::Vector value) {
	ns3::Ptr<ns3::MobilityModel> m = m_node->GetObject<ns3::MobilityModel > ();
    m->SetPosition(value);
}

ns3::Ipv4Address Agent::GetAddress() {
    return iic.GetAddress(0);
}

ns3::Address Agent::GetBroadcastAddress() {
    return m_device->GetBroadcast();
}

int Agent::SendTo(ns3::Ptr<Agent> agent, ns3::Ptr<ns3::Packet> packet) {
	if(!agent)
	{
		NS_LOG_UNCOND("Destination Agent is Invalid");
		return -1;
	}
	ns3::InetSocketAddress remote = ns3::InetSocketAddress (agent->GetAddress(), 80);
	m_socket->Connect (remote);
		  NS_LOG_UNCOND ("Sending packer[" << sim_mob::Broker::global_pckt_cnt++ << "] from Address[" << m_AgentId << " : " << GetAddress() << "] to remote location[" << agent->GetAgentId() << " : "  << agent->GetAddress() << "]");
	int res = m_socket->Send(packet);
    return res;
}

/*************************************************************************************
 * ************************************************************************************
 * ************************************************************************************
 * ************************************************************************************
 */

//and apply this to the base class
ns3::NodeContainer WFD_Agent::wfd_nc;
ns3::NetDeviceContainer WFD_Agent::wfd_ndc ;
std::map<unsigned int , ns3::NodeContainer>  WFD_Agent::wfd_go_nc;
std::map<unsigned int , ns3::NetDeviceContainer>  WFD_Agent::wfd_go_ndc ;
std::map<unsigned int , ns3::NodeContainer>  WFD_Agent::wfd_cln_nc;
std::map<unsigned int , ns3::NetDeviceContainer>  WFD_Agent::wfd_cln_ndc ;
WifiHelper WFD_Agent::wfd_wifi;
NqosWifiMacHelper WFD_Agent::wfd_mac = NqosWifiMacHelper::Default ();
YansWifiPhyHelper WFD_Agent::wfd_wifiPhy = YansWifiPhyHelper::Default ();
YansWifiChannelHelper WFD_Agent::wfd_wifiChannel = YansWifiChannelHelper::Default ();
ns3::InternetStackHelper WFD_Agent::wfd_internet;
ns3::MobilityHelper WFD_Agent::wfd_mobility;
ns3::Ptr<ListPositionAllocator> WFD_Agent::wfd_positionAlloc;
ns3::Ipv4AddressHelper WFD_Agent::wfd_ipAddrs;

WFD_Agent::AgentRole WFD_Agent::getRole(){
	return role;
}

void WFD_Agent::setRole(AgentRole role_){
	role = role_;
}

WFD_Agent::WFD_Agent(int m_AgentId_, sim_mob::Broker* broker_):Agent(m_AgentId_,broker_){
	NS_LOG_UNCOND("Inside WFD_Agent::WFD_Agent(2)");
}
WFD_Agent::WFD_Agent():Agent(){
	NS_LOG_UNCOND("Inside WFD_Agent::WFD_Agent(0)" << this);
}
//per agent initialization
void WFD_Agent::init(){

	NS_LOG_UNCOND("Inside WFD_Agent::init");
	m_wfd_node = ns3::CreateObject<ns3::Node> ();
    //
    // fill in proper container to manage the nodes of the LAN.  We need
    // at least three containers here; one with all of the client nodes,
	// one with GO and one with all of the nodes including go and client nodes
	//todo, this function and doRoleAssignment in WFD_registration are doing the same thing. must cancel one and use the results of the other one
	figureOutGroup();//trivial function to ,later , support multi roups

	if(getRole() == WFD_GO){
		wfd_go_nc[groupId].Add(m_wfd_node);
	}
	else if(getRole() == WFD_CLIENT){
		wfd_cln_nc[groupId].Add(m_wfd_node);
	}

	wfd_nc.Add(m_wfd_node);

}
ns3::Ptr<Agent> WFD_Agent::clone(int m_AgentId_ , sim_mob::Broker* broker_){

//	NS_LOG_UNCOND("Inside WFD_Agent::clone" << this);
	return ns3::CreateObject<WFD_Agent>(m_AgentId_,broker_);
}

void WFD_Agent::figureOutGroup(){
	groupId = 0;//for now
	setRole(((this->GetAgentId() == m_all_agents.begin()->second->GetAgentId()) ? WFD_GO : WFD_CLIENT));
}

//all agents configuration
void WFD_Agent::configAll(){
	  // Reset the address base--the networks will be in
	  // the "10.0" address space
	wfd_ipAddrs.SetBase ("10.0.0.0", "255.255.255.0");
	  for (uint32_t groupId_ = 0; groupId_ < wfd_go_nc.size() ; ++groupId_)
	    {
	      //
	      // Create an infrastructure network
	      //
	      wfd_wifi = WifiHelper::Default ();
	      wfd_mac = NqosWifiMacHelper::Default ();
	      wfd_wifiPhy.SetChannel (wfd_wifiChannel.Create ());
	      // Create unique ssids for these networks
	      std::string ssidString ("WFD-GRP-");
	      std::stringstream ss;
	      ss << groupId_;
	      ssidString += ss.str ();
	      ns3::Ssid ssid = Ssid (ssidString);
	      wfd_wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
	      // setup stas
	      wfd_mac.SetType ("ns3::StaWifiMac",
	                        "Ssid", SsidValue (ssid),
	                        "ActiveProbing", BooleanValue (false));
	      wfd_cln_ndc[groupId_] = wfd_wifi.Install (wfd_wifiPhy, wfd_mac, wfd_cln_nc[groupId_]);
	      // setup ap.
	      wfd_mac.SetType ("ns3::ApWifiMac",
	                        "Ssid", SsidValue (ssid));
	      wfd_go_ndc[groupId_] = wfd_wifi.Install (wfd_wifiPhy, wfd_mac, wfd_go_nc[groupId_].Get(0));
	      // Collect all of these new devices and nodes
	      NetDeviceContainer grpDevices (wfd_go_ndc[groupId_], wfd_cln_ndc[groupId_]);
	      NodeContainer grpNodes (wfd_go_nc[groupId_], wfd_cln_nc[groupId_]);
	      //d
	      // Add the IPv4 protocol stack to the nodes in our container
	      //
	      NS_LOG_UNCOND("installing internet stack for " << grpNodes.GetN() << " nodes\n");
//	      InternetStackHelper internet;
//	      internet.Install(grpNodes);

	      wfd_internet.Install (grpNodes);
	      NS_LOG_UNCOND("installing internet stack for " << grpNodes.GetN() << " nodes done\n");
	      //
	      // Assign IPv4 addresses to the device drivers (actually to the associated
	      // IPv4 interfaces) we just created.
	      //
	      Ipv4InterfaceContainer wfd_addresses = wfd_ipAddrs.Assign (grpDevices);
	      Ipv4InterfaceContainer::Iterator it;
	      for(it = wfd_addresses.Begin(); it != wfd_addresses.End(); it++){
	    	  NS_LOG_UNCOND(it->first->GetAddress(1,0));
	      }
	      //
	      // Assign a new network prefix for each mobile network, according to
	      // the network mask initialized above
	      //
	      wfd_ipAddrs.NewNetwork ();
//	      //
//	      // The new wireless nodes need a mobility model so we aggregate one
//	      // to each of the nodes we just finished building.
//	      //
//	      Ptr<ListPositionAllocator> subnetAlloc =
//	        CreateObject<ListPositionAllocator> ();
//	      for (uint32_t j = 0; j < grpNodes.GetN (); ++j)
//	        {
//	          subnetAlloc->Add (Vector (0.0, j, 0.0));
//	        }
	      wfd_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	      wfd_mobility.Install (grpNodes);
	    }

}
} /* namespace sim_mob */


