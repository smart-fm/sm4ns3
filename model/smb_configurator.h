/*
 * Configurator.h
 *
 *  Created on: Nov 21, 2013
 *      Author: vahid
 */
#pragma once

#include<string>


namespace sim_mob {

class Configurator {
public:
	std::string context;
	Configurator();
	virtual void configure();
	virtual ~Configurator();
};

} /* namespace sim_mob */
