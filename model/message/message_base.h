//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

namespace sm4ns3 {

//New base message class; contains everything a Message is guranteed to have. 
//Does NOT support dynamic inheritance; never store a subclass using this class as a pointer.
struct MessageBase {
	std::string msg_type;    ///<The "type" of this message. Used to identify the subclass.
};

//TODO: This might need to go somewhere else.
struct DPoint {
	double x;
	double y;
	DPoint(double x=0.0, double y=0.0) : x(x),y(y) {}
};

}

