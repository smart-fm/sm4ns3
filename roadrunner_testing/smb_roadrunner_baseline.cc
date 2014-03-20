#include "smb_roadrunner_baseline.h"

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


sm4ns3::RoadRunnerBaseLine::RoadRunnerBaseLine(){
	MessageMap = boost::assign::map_list_of
			("MULTICAST", MULTICAST)
			("add_agent", RoadRunnerBaseLine::ADD)
			("update_agent", RoadRunnerBaseLine::UPDATE);
}

void sm4ns3::RoadRunnerBaseLine::parse() {
	std::string line;
	while (1/*std::getline(infile, line)*/) {

		std::vector<std::string> tokens;
		boost::split(tokens, line, boost::is_any_of(";"));
		std::string timeStr = tokens[1].substr(5,tokens[1].find("ms"));
		unsigned int time = boost::lexical_cast<unsigned int>(timeStr);
		time++;
		switch(MessageMap[tokens[0]]){
		case ADD:
		break;
		case UPDATE:
		break;
		case MULTICAST:
		break;

		}
	}
}


bool sm4ns3::RoadRunnerBaseLine::start(std::string fileName){
	//step-1 open file
	infile.open(fileName.c_str(), std::ifstream::in);
	//step-2 schedule now(tick 0)
	return true;
}

void sm4ns3::RoadRunnerBaseLine::pause(){
	//step-1 read line and parse(until the next time tick arrives)-|
	//step-2 put the parses into queue-----------------------------|
	//step-3 process queue
	//step-3 schedule the next tick
}
