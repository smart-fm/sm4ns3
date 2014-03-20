//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once 

#include <sstream>
#include <string>
#include <fstream>

#include "smb_broker.h"

namespace sm4ns3 {

class RoadRunnerBaseLine : sm4ns3::Broker {
	enum messageType{
		MULTICAST,
		ADD,
		UPDATE
	};
	std::map<std::string, messageType> MessageMap;

	std::ifstream infile;
	void readLine();
	void parseLine();
public:
	RoadRunnerBaseLine();

	void parse();
	bool start(std::string parameter = "");
	virtual void pause();
};

}

