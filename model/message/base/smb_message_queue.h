/*
 * MessageQueue.h
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#pragma once

#include "smb_message_base.h"
#include "ns3/system-mutex.h"

#include <queue>

using namespace boost;

namespace sm4ns3 {

template<class T>
class MessageQueue {
	std::queue<T> messageList;
	ns3::SystemMutex mutex;
	//boost::shared_mutex mutex;
public:
	MessageQueue();
	virtual ~MessageQueue();
	bool ReadMessage();
    void post(T message);
    bool pop(T&);
    int size();
};

template<class T>
MessageQueue<T>::~MessageQueue(){


}

template<class T>
bool MessageQueue<T>::ReadMessage(){
	//boost::unique_lock< boost::shared_mutex > lock(mutex);
	ns3::CriticalSection lock(mutex);
	return true;
}

template<class T>
void MessageQueue<T>::post(T message){
	//boost::unique_lock< boost::shared_mutex > lock(mutex);
	ns3::CriticalSection lock(mutex);
	messageList.push(message);
}

template<class T>
bool MessageQueue<T>::pop(T &t ){
	//boost::unique_lock< boost::shared_mutex > lock(mutex);
	ns3::CriticalSection lock(mutex);
	if(messageList.empty())
	{
		return false;
	}
	t = messageList.front();
	messageList.pop();
	return true;
}


template<class T>
int MessageQueue<T>::size() {
	return messageList.size();
}

template<class T>
MessageQueue<T>::MessageQueue() {
	// TODO Auto-generated constructor stub

}

} /* namespace sm4ns3 */
