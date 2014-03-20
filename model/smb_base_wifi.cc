/*
 * BaseWifi.cc
 *
 *  Created on: Jul 30, 2013
 *      Author: vahid
 */

#include "smb_base_wifi.h"
//#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"

namespace sm4ns3 {

BaseWifi::BaseWifi() {
//	NS_LOG_UNCOND("inside BaseWifi constructor");
	m_init_done = false;
	m_rss = -80;
	m_phyMode ="DsssRate1Mbps";
	// TODO Auto-generated constructor stub
	init();
}

//BaseWifi::BaseWifi(const BaseWifi &wifi)
//{
//
////	NS_LOG_UNCOND("inside BaseWifi copy constructor");
//    m_wifiHelper = wifi.m_wifiHelper; // a wifi helper apply to setup vehicles Wifi
//
//    m_wifiMacHelper = wifi.m_wifiMacHelper; // a wifi mac helper apply to setup vehicles Wifi
//
//    m_wifiPhyHelper = wifi.m_wifiPhyHelper; // a wifi phy helper apply to setup vehicles Wifi
//
//    m_phyMode = wifi.m_phyMode;
//
//    m_rss = wifi.m_rss;  // -dBm
//
//    m_init_done = wifi.m_init_done;
////    BaseWifi();
//}

//BaseWifi& BaseWifi::operator =(const BaseWifi &wifi)
//{
//
////	NS_LOG_UNCOND("inside BaseWifi Assignment operator");
//    m_wifiHelper = wifi.m_wifiHelper; // a wifi helper apply to setup vehicles Wifi
//
//    m_wifiMacHelper = wifi.m_wifiMacHelper; // a wifi mac helper apply to setup vehicles Wifi
//
//    m_wifiPhyHelper = wifi.m_wifiPhyHelper; // a wifi phy helper apply to setup vehicles Wifi
//
//    m_phyMode = wifi.m_phyMode;
//
//    m_rss = wifi.m_rss;  // -dBm
//
//    m_init_done = wifi.m_init_done;
////    BaseWifi();
//    return *this;
//}


void BaseWifi::init() {
//	NS_LOG_UNCOND("inside BaseWifi::init()");
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

	  m_wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
	                                "DataMode",ns3::StringValue (m_phyMode),
	                                "ControlMode",ns3::StringValue (m_phyMode));
	  // Set it to adhoc mode
	  m_wifiMacHelper.SetType ("ns3::AdhocWifiMac");

	  m_init_done = true;
}

//Install the class's embedded settings on the nodes in this node container.
ns3::NetDeviceContainer BaseWifi::Install(ns3::NodeContainer &nc) {
	return m_wifiHelper.Install(m_wifiPhyHelper, m_wifiMacHelper, nc);
}

void BaseWifi::configure() {
//	if (type == APP_ROADRUNNER) {//todo switch case

		//	NS_LOG_UNCOND("inside BaseWifi::init()");
		m_wifiHelper.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);

		m_wifiPhyHelper = ns3::YansWifiPhyHelper::Default();

		// This is one parameter that matters when using FixedRssLossModel
		// set it to zero; otherwise, gain will be added
		m_wifiPhyHelper.Set("RxGain", ns3::DoubleValue(0));

		// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
		m_wifiPhyHelper.SetPcapDataLinkType(
				ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

		ns3::YansWifiChannelHelper wifiChannel;

		wifiChannel.SetPropagationDelay(
				"ns3::ConstantSpeedPropagationDelayModel");

		// The below FixedRssLossModel will cause the rss to be fixed regardless
		// of the distance between the two stations, and the transmit power
		wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss",
				ns3::DoubleValue(m_rss));

		m_wifiPhyHelper.SetChannel(wifiChannel.Create());

		// Add a non-QoS upper mac, and disable rate control
		m_wifiMacHelper = ns3::NqosWifiMacHelper::Default();

		m_wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
				"DataMode", ns3::StringValue(m_phyMode), "ControlMode",
				ns3::StringValue(m_phyMode));
		// Set it to adhoc mode
		m_wifiMacHelper.SetType("ns3::AdhocWifiMac");

		m_init_done = true;
//	}
}

//void BaseWifi::EnablePcap (/*"wifi-simple-adhoc"*/std::string str, ns3::NetDeviceContainer &devices) {
//	m_wifiPhyHelper.EnablePcap(str, devices);
//}

BaseWifi::~BaseWifi() {
	// TODO Auto-generated destructor stub
}

} /* namespace sm4ns3 */
