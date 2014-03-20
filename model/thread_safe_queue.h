//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "message_base.h"
#include "ns3/system-mutex.h"

#include <queue>

namespace sm4ns3 {

/**
 * Implements a simple queue that is thread-safe for all individual operations.
 * Use push() and pop() to add/remove members to/from the queue.
 * These operations are locked with a mutex.
 */
template<class T>
class ThreadSafeQueue {
public:
	///Push an item into the queue.
	void push(const T& item);

	///Pop an item off the queue and store it in res. 
	///Returns true if an item was retrieved, false otherwise.
	bool pop(T& res);

	///Returns the size of the queue
	int size() const;

private:
	std::queue<T> messageList;
	ns3::SystemMutex mutex;
};

} //End sm4ns3


//////////////////////////////////////////////////////////
// Template implementation
//////////////////////////////////////////////////////////

template<class T>
void sm4ns3::ThreadSafeQueue<T>::push(const T& item)
{
	ns3::CriticalSection lock(mutex);

	messageList.push(item);
}

template<class T>
bool sm4ns3::ThreadSafeQueue<T>::pop(T &res)
{
	ns3::CriticalSection lock(mutex);

	if(!messageList.empty()) {
		res = messageList.front();
		messageList.pop();
		return true;
	}
	return false;
}


template<class T>
int sm4ns3::ThreadSafeQueue<T>::size() const
{
	ns3::CriticalSection lock(mutex);

	return messageList.size();
}

