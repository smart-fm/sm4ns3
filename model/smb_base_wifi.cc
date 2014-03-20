//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "smb_base_wifi.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"


sm4ns3::BaseWifi::BaseWifi() : m_phyMode("DsssRate1Mbps"), 	m_rss(-80), m_init_done(false)
{
	init();
}


void sm4ns3::BaseWifi::init() 
{
	m_wifiHelper.SetStandard (ns3::WIFI_PHY_STANDARD_80211b);
	m_wifiPhyHelper =  ns3::YansWifiPhyHelper::Default ();

	// This is one parameter that matters when using FixedRssLossModel
	// set it to zero; otherwise, gain will be added
	m_wifiPhyHelper.Set ("RxGain", ns3::DoubleValue (0) );

	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	m_wifiPhyHelper.SetPcapDataLinkType (ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
	ns3::YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");

	// The below FixedRssLossModel will cause the rss to be fixed regardless
	// of the distance between the two stations, and the transmit power
	wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",ns3::DoubleValue (m_rss));
	m_wifiPhyHelper.SetChannel (wifiChannel.Create ());

	// Add a non-QoS upper mac, and disable rate control
	m_wifiMacHelper = ns3::NqosWifiMacHelper::Default ();
	m_wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",ns3::StringValue (m_phyMode), "ControlMode",ns3::StringValue (m_phyMode));

	// Set it to adhoc mode
	m_wifiMacHelper.SetType ("ns3::AdhocWifiMac");
	m_init_done = true;
}


ns3::NetDeviceContainer sm4ns3::BaseWifi::Install(ns3::NodeContainer &nc) 
{
	return m_wifiHelper.Install(m_wifiPhyHelper, m_wifiMacHelper, nc);
}

void sm4ns3::BaseWifi::configure() 
{
	m_wifiHelper.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);
	m_wifiPhyHelper = ns3::YansWifiPhyHelper::Default();

	// This is one parameter that matters when using FixedRssLossModel
	// set it to zero; otherwise, gain will be added
	m_wifiPhyHelper.Set("RxGain", ns3::DoubleValue(0));

	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	m_wifiPhyHelper.SetPcapDataLinkType(ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
	ns3::YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

	// The below FixedRssLossModel will cause the rss to be fixed regardless
	// of the distance between the two stations, and the transmit power
	wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", ns3::DoubleValue(m_rss));
	m_wifiPhyHelper.SetChannel(wifiChannel.Create());

	// Add a non-QoS upper mac, and disable rate control
	m_wifiMacHelper = ns3::NqosWifiMacHelper::Default();
	m_wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", ns3::StringValue(m_phyMode), "ControlMode", ns3::StringValue(m_phyMode));

	// Set it to adhoc mode
	m_wifiMacHelper.SetType("ns3::AdhocWifiMac");
	m_init_done = true;
}


