/*
 * BaseWifi.h
 *
 *  Created on: Jul 30, 2013
 *      Author: vahid
 */

#pragma once

//#include "ns3/core-module.h"
#include "ns3/wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "smb_configurator.h"

namespace sim_mob {

class BaseWifi : public Configurator {
    ns3::WifiHelper m_wifiHelper; // a wifi helper apply to setup vehicles Wifi

    ns3::NqosWifiMacHelper m_wifiMacHelper; // a wifi mac helper apply to setup vehicles Wifi

    ns3::YansWifiPhyHelper m_wifiPhyHelper; // a wifi phy helper apply to setup vehicles Wifi

    std::string m_phyMode;

    double m_rss;  // -dBm

    bool m_init_done;

public:

	BaseWifi();

//	BaseWifi(const BaseWifi &wifi);

//	BaseWifi& operator =(const BaseWifi &wifi);

	virtual void init();

	ns3::NetDeviceContainer Install(ns3::NodeContainer &c);
//	void EnablePcap (/*"wifi-simple-adhoc"*/std::string str, ns3::NetDeviceContainer &devices);
	void configure();
	virtual ~BaseWifi();
};

} /* namespace sim_mob */
