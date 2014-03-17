/*
 * MessageFactory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "ns3/log.h"

namespace sim_mob {

template <class RET>
class BaseFactory {
	std::map<std::string, const RET> prototypes;
public:
	void Register(std::string name, const RET prototype)
	{
		if (prototypes.count(name)>0) {
			std::string str ;
			str = "Duplicate type:" + name;
			throw std::runtime_error(str);
		}
		prototypes.insert (std::make_pair(name,prototype));
	}
	const RET getPrototype(const std::string& name, bool &success = false) const
	{
		success = false;
		typename std::map<std::string, const RET>::const_iterator it = prototypes.find(name);
		if (it!=prototypes.end()) {
			//NS_LOG_UNCOND( name << " found in the prototypes\n";
			const RET prototype = it->second;
			NS_LOG_UNCOND( "getPrototype succeeded [" << name << "]" );
			success = true;
			return prototype;
		}
		NS_LOG_UNCOND( "getPrototype Failed [" << name << "]" );
		return RET();
	}
};

template <class MSG,class RET, class DT> //MSG: received message type(exp:string) , RET : parsed output type(exp: Mesage), DT: Message data's type(ythe data type of the data member in Message class)
class MessageFactory {
	std::map<std::string, const RET> prototypes;
public:
	virtual bool createMessage(MSG,std::vector<RET>&) = 0;
	virtual bool createMessage(DT&,RET&) = 0;
	void registerMessage(std::string name, const RET prototype)
	{
		if (prototypes.count(name)>0) {
			std::string str ;
			str = "Duplicate Message type:" + name;
			throw std::runtime_error(str);
		}
		prototypes.insert (std::make_pair(name,prototype));
	}
	const RET getPrototype(const std::string& name, bool &success = false) const
	{
		success = false;
		typename std::map<std::string, const RET>::const_iterator it = prototypes.find(name);
		if (it!=prototypes.end()) {
			//NS_LOG_UNCOND( name << " found in the prototypes\n";
			const RET prototype = it->second;
//			NS_LOG_UNCOND( "getPrototype succeeded [" << name << "]" );
			success = true;
			return prototype;
		}
		NS_LOG_UNCOND( "getPrototype Failed [" << name << "]" );
		return RET();
	}
};

} /* namespace sim_mob */
