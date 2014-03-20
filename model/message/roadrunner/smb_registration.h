//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "smb_message_factory_base.h"

#include <iostream>
#include <boost/shared_ptr.hpp>

namespace sm4ns3 {
//Forward declaration
class Connection;
struct msg_header;
class Broker;

class Registration {
//	static sm4ns3::BaseFactory<Registration*> m_appRegFactory;
protected:
	std::string m_application;
	sm4ns3::Broker* m_broker;
	std::string m_simmobility_address;
	std::string m_simmobility_port;
public:
	Registration(sm4ns3::Broker* broker_, std::string simmobility_address_, std::string simmobility_port_, std::string application = "Default");
	virtual ~Registration();
	virtual Registration * clone()const;
	bool doConnect();
	bool doWhoAreYou();
	bool doAGENTS_INFO();
	bool doREADY();
	virtual bool doAgentInit();
	virtual bool start();
};

class WFD_Registration : public Registration {

	struct WFD_Group{
		int groupId;//might come handy
		unsigned int GO;
		std::vector<unsigned int> members;//clients and GO
		WFD_Group(unsigned int groupId,unsigned int GO,
				std::vector<unsigned int> members):
					groupId(groupId),GO(GO), members(members)
		{}
		WFD_Group(){}
	};

	///	keeps the record of wfd groups in simmobility
	std::map<unsigned int, WFD_Group> WFD_Groups_; //<groupId, WFD_Group>
	///	separate container that associates an agent to a wfd group
	std::map<unsigned int, unsigned int> WFD_Membership; //<agent id, groupId>


	/**
	 * Performs a group owner negotiation
	 */
	bool doRoleAssignment();

	/**
	 * Create a json string of GO/Client information
	 * based on local containers
	 */
	std::string makeGO_ClientPacket();

	//Serialize part of a GOCLIENT message.
	static void makeGO_ClientArrayElement(unsigned int go, std::vector<unsigned int> clients, Json::Value & output);

public:
	WFD_Registration(sm4ns3::Broker*,
			std::string simmobility_address_,
			std::string simmobility_port_, std::string application_ = "stk");
	virtual ~WFD_Registration();
	/**
	 * All in one WFD configuration method
	 * send back the configuration results to simobility
	 */
	bool WFD_Configuration();
	/*
	 * starts the registration process for this type of application
	 */
	bool start();
//	/*
//	 * Initializes the registered agents as the base or derived class pleases
//	 */
//	bool doAgentInit();
	Registration * clone()const;
};

} /* namespace sm4ns3 */
