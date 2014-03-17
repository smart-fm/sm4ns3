/*
 * BaseMobility.cc
 *
 *  Created on: Aug 6, 2013
 *      Author: vahid
 */

#include "smb_base_mobility.h"
#include "ns3/position-allocator.h"
#include "ns3/node-container.h"

namespace sim_mob {

BaseMobility::BaseMobility() {
	// TODO Auto-generated constructor stub

}

void BaseMobility::init() {
	  // Note that with FixedRssLossModel, the positions below are not
	  // used for received signal strength.

//	  positionAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
//	  positionAlloc->Add (ns3::Vector (0.0, 0.0, 0.0));
//	  positionAlloc->Add (ns3::Vector (5.0, 0.0, 0.0));
//	  m_mobility.SetPositionAllocator (positionAlloc);
//	  m_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

}
void BaseMobility::configure(){

}

void BaseMobility::Install(ns3::NodeContainer &nc) {
	m_mobility.Install (nc);
}

BaseMobility::~BaseMobility() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
