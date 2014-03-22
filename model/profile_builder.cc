//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "profile_builder.h"

#include <iostream>
#include <stdexcept>

using namespace sm4ns3;

using std::string;

//Static initialization
std::ofstream ProfileBuilder::LogFile;
boost::mutex ProfileBuilder::profile_mutex;
int ProfileBuilder::ref_count = 0;


void ProfileBuilder::InitLogFile(const string& path)
{
	LogFile.open(path.c_str());
	if (LogFile.fail()) {
		throw std::runtime_error("Couldn't open Profile Builder log file.");
	}
}


ProfileBuilder::ProfileBuilder()
{
	RefCountUpdate(1);
}

ProfileBuilder::~ProfileBuilder()
{
	//Write any pending output
	flushLogFile();

	//Close the log file?
	int numLeft = RefCountUpdate(-1);
	if (numLeft==0) {
		//We're the only one left; let the log file know we have truly captured all the output.
		LogFile <<"ProfileBuilder RefCount reached zero.\n";
		LogFile.close();
	}
}

int ProfileBuilder::RefCountUpdate(int amount)
{
	{
	boost::mutex::scoped_lock local_lock(profile_mutex);
	ProfileBuilder::ref_count += amount;
	return ProfileBuilder::ref_count;
	}
}

void ProfileBuilder::flushLogFile()
{
	//Lock it all
	boost::mutex::scoped_lock local_lock(profile_mutex);

	//Do nothing if the current log string is empty.
	std::string currLogStr = currLog.str();
	if (currLogStr.empty()) {
		return;
	}

	//Error if the log file isn't in a valid state.
	if (!(LogFile.is_open() && LogFile.good())) {
		throw std::runtime_error("ProfileBuilder can't flush log file; log file is not open.");
	}

	//Write and flush the current buffer.
	LogFile <<currLogStr;
	LogFile.flush();
	currLog.str("");
}

string ProfileBuilder::GetCurrentTime()
{
	timespec timeres;
	int res;

	{
	//The documentation claims that clock_gettime() is thread-safe
	//boost::mutex::scoped_lock local_lock(profile_mutex);
	res = clock_gettime(CLOCK_REALTIME, &timeres); //For epoch time.
	//res = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timeres); //For time since the process started
	}

	//Error?
	if (res!=0) {
		return "<error>";
	}

	//Convert
	std::stringstream msg;
	msg <<"(sec," <<timeres.tv_sec <<"),";
	msg <<"(nano," <<timeres.tv_nsec <<")";
	return msg.str();
}



void ProfileBuilder::logSiMobilityBegin()
{
	logGeneric("simmob-loop-begin-main-loop");
}


void ProfileBuilder::logSiMobilityEnd()
{
	logGeneric("simmob-loop-end-main-loop");
}


void ProfileBuilder::logTickBegin(int currFrame, int numAgents, int numCommunications)
{
	logGeneric("simmob-tick-begin", currFrame, numAgents, numCommunications);
}


void ProfileBuilder::logTickEnd(int currFrame, int numAgents, int numCommunications)
{
	logGeneric("simmob-tick-end", currFrame, numAgents, numCommunications);
}



void ProfileBuilder::logGeneric(const std::string& itemName, int currFrame, int numAgents, int numCommunications)
{
	//Mandatory properties.
	currLog <<"{"
			<<"\"" <<"action" <<"\""    <<":" <<"\"" <<itemName <<"\"" <<","
			<<"\"" <<"real-time" <<"\"" <<":" <<"\"" <<GetCurrentTime() <<"\"" <<",";

	//Optional
	if (currFrame>=0) {
		currLog <<"\"" <<"tick" <<"\""      <<":" <<"\"" <<currFrame <<"\"" <<",";
		currLog <<"\"" <<"num-agents" <<"\""      <<":" <<"\"" <<numAgents <<"\"" <<",";
		currLog <<"\"" <<"num-communications" <<"\""      <<":" <<"\"" <<numCommunications <<"\"" <<",";
	}

	//Close it.
	currLog <<"}\n";
}


