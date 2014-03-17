/*
 * BaseMobility.h
 *
 *  Created on: Aug 6, 2013
 *      Author: vahid
 */

#pragma once

#include "ns3/mobility-helper.h"
#include "smb_configurator.h"

namespace ns3{
class NodeContainer;
class ListPositionAllocator;
}
namespace sim_mob {

class BaseMobility : public Configurator {
	ns3::MobilityHelper m_mobility;
	 ns3::Ptr<ns3::ListPositionAllocator> positionAlloc ;
public:
	BaseMobility();

	void init();
	virtual void configure();

	void Install(ns3::NodeContainer &nc);
	virtual ~BaseMobility();
};

} /* namespace sim_mob */
