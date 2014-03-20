//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "ns3/mobility-helper.h"

namespace ns3 {
class NodeContainer;
class ListPositionAllocator;
} //End namespace ns3


namespace sm4ns3 {
class BaseMobility  {
public:
	void init();
	void Install(ns3::NodeContainer &nc);

private:
	ns3::MobilityHelper m_mobility;
	ns3::Ptr<ns3::ListPositionAllocator> positionAlloc;
};

}

