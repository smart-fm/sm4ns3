#include "smb_broker.h"

#include <sstream>
#include <string>
#include <fstream>

namespace sim_mob {

class RoadRunnerBaseLine : sim_mob::Broker {
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

}//namespace
