#include "smb_time_info.h"
//#include "smb_message_info.h"
#include "smb_agent.h"
#include "smb_broker.h"
namespace sim_mob {
MSG_Time::MSG_Time(msg_data_t data_): Message(data_)
{

}
MSG_Time::MSG_Time()
{

}

MSG_Time::~MSG_Time()
{

}

/*sim_mob::comm::Message<msg_data_t> **/ msg_ptr  MSG_Time::clone(msg_data_t& data_) {
	return msg_ptr (new MSG_Time(data_));
}

Handler * MSG_Time::newHandler()
{
	return new HDL_Time();
}

void HDL_Time::handle(msg_ptr message_,Broker* broker){
	//for now, just set a global tick counter in the broker

}//handle()

HDL_Time::~HDL_Time() {

}
}//namespace
