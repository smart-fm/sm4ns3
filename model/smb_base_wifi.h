//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "ns3/wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/yans-wifi-helper.h"

namespace sm4ns3 {

class BaseWifi {
public:
	BaseWifi();
	virtual void init();
	ns3::NetDeviceContainer Install(ns3::NodeContainer &c);
	void configure();

private:
	ns3::WifiHelper m_wifiHelper; // a wifi helper apply to setup vehicles Wifi
	ns3::NqosWifiMacHelper m_wifiMacHelper; // a wifi mac helper apply to setup vehicles Wifi
	ns3::YansWifiPhyHelper m_wifiPhyHelper; // a wifi phy helper apply to setup vehicles Wifi
	std::string m_phyMode;
	double m_rss;  // -dBm
	bool m_init_done;

};

}

