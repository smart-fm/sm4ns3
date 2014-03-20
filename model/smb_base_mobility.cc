//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "smb_base_mobility.h"
#include "ns3/position-allocator.h"
#include "ns3/node-container.h"

void sm4ns3::BaseMobility::init() 
{
	  // Note that with FixedRssLossModel, the positions below are not
	  // used for received signal strength.
//	  positionAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
//	  positionAlloc->Add (ns3::Vector (0.0, 0.0, 0.0));
//	  positionAlloc->Add (ns3::Vector (5.0, 0.0, 0.0));
//	  m_mobility.SetPositionAllocator (positionAlloc);
//	  m_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

}

void sm4ns3::BaseMobility::Install(ns3::NodeContainer &nc) 
{
	m_mobility.Install (nc);
}

