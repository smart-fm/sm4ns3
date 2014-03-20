//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <jsoncpp/json/json.h>

namespace sm4ns3 {

//Forward declaration
class Broker;

class Registration {
public:
	Registration(sm4ns3::Broker* broker_, std::string application = "Default");
	virtual ~Registration();
	virtual bool start();

protected:
	virtual bool doInitialization();

	//Attempts to connect to the server, until timeout seconds have passed
	bool doConnect(unsigned int timeout=30);

	bool doWhoAreYou();

	bool doAGENTS_INFO();

	bool doREADY();

protected:
	std::string m_application;
	sm4ns3::Broker* m_broker;
	std::string m_simmobility_address;
	std::string m_simmobility_port;
};


class WFD_Registration : public Registration {
public:
	WFD_Registration(sm4ns3::Broker*, std::string application_ = "stk");
	virtual ~WFD_Registration();

protected:
	///All in one WFD configuration method send back the configuration results to simobility
	virtual bool doInitialization();

private:
	struct WFD_Group{
		int groupId;//might come handy
		unsigned int GO;
		std::vector<unsigned int> members;//clients and GO
		WFD_Group(unsigned int groupId,unsigned int GO, std::vector<unsigned int> members) : groupId(groupId),GO(GO), members(members) {}
		WFD_Group(){}
	};

	///Keeps the record of wfd groups in simmobility
	std::map<unsigned int, WFD_Group> WFD_Groups_; //<groupId, WFD_Group>

	///Separate container that associates an agent to a wfd group
	std::map<unsigned int, unsigned int> WFD_Membership; //<agent id, groupId>

	///Performs a group owner negotiation
	bool doRoleAssignment();

	///Create a json string of GO/Client information based on local containers
	std::string makeGO_ClientPacket();

	///Serialize part of a GOCLIENT message.
	static void makeGO_ClientArrayElement(unsigned int go, std::vector<unsigned int> clients, Json::Value & output);
};

}




