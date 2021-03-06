//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <jsoncpp/json/json.h>

namespace sm4ns3 {

//Forward declaration
class BrokerBase;
class BundleHeader;

struct WFD_Group{
	std::string groupId;//might come handy
	std::string GO;
	std::vector<std::string> members;//clients and GO
	WFD_Group(const std::string& groupId, const std::string& GO, std::vector<std::string> members) : groupId(groupId),GO(GO), members(members) {}
	WFD_Group(){}
};

class Registration {
public:
	Registration(BrokerBase* broker, std::string app = "Default");
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
	BrokerBase* broker;
	std::string m_simmobility_address;
	std::string m_simmobility_port;
};


class WFD_Registration : public Registration {
public:
	WFD_Registration(BrokerBase* broker, std::string app = "stk");
	virtual ~WFD_Registration();

protected:
	///All in one WFD configuration method send back the configuration results to simobility
	virtual bool doInitialization();

private:
	///Keeps the record of wfd groups in simmobility
	std::map<std::string, WFD_Group> WFD_Groups_; //<groupId, WFD_Group>

	///Separate container that associates an agent to a wfd group
	std::map<std::string, std::string> WFD_Membership; //<agent id, groupId>

	///Performs a group owner negotiation
	bool doRoleAssignment();
};

}




