//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include <boost/thread/mutex.hpp>


namespace sm4ns3 {


///A trimmed-down version of Sim Mobility's profile builder. Used for buffering of large amounts of nanoscale timing updates.
class ProfileBuilder {
	timeval currTime[2];
public:
	ProfileBuilder();
	~ProfileBuilder();

	///Initialize the shared log file. Must be called once before any output is written.
	static void InitLogFile(const std::string& path);

	void logSiMobilityBegin();
	void logSiMobilityEnd();

	void logTickBegin(int currFrame, int numAgents, int numCommunications);
	void logTickEnd(int currFrame, int numAgents, int numCommunications);

	//If currFrame is <0, we don't log those three remaining properties.
	void logGeneric(const std::string& itemName, int currFrame=-1, int numAgents=-1, int numCommunications=-1);


private:
	static std::string GetCurrentTime();
	static int RefCountUpdate(int amount);
	void flushLogFile();

private:
	//Used for maintaining the shared log file.
	static boost::mutex profile_mutex;
	static std::ofstream LogFile;
	static int ref_count;

	//Local buffer
	std::stringstream currLog;
};

}



