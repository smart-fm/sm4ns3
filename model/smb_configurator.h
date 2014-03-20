/*
 * Configurator.h
 *
 *  Created on: Nov 21, 2013
 *      Author: vahid
 */
#pragma once

#include<string>


namespace sm4ns3 {

class Configurator {
public:
	std::string context;
	Configurator();
	virtual void configure();
	virtual ~Configurator();
};

} /* namespace sm4ns3 */
