#include "smb_time_info.h"
//#include "smb_message_info.h"
#include "smb_agent.h"
#include "smb_broker.h"
namespace sim_mob {
MSG_Time::MSG_Time(const Json::Value& data_, const sim_mob::msg_header& header): Message(data_, header)
{
}

/*MSG_Time::MSG_Time()
{
}*/

MSG_Time::~MSG_Time()
{
}


/*Handler * MSG_Time::newHandler()
{
	return new HDL_Time();
}*/

void HDL_Time::handle(msg_ptr message_,Broker* broker) const 
{
	//for now, just set a global tick counter in the broker

}//handle()

HDL_Time::~HDL_Time() {

}
}//namespace
