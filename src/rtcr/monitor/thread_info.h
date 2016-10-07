/*
 * \brief  Monitoring thread information
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_THREAD_INFO_COMPONENT_H_
#define _RTCR_THREAD_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <thread/capability.h>

namespace Rtcr {
	struct Thread_info;
}

/**
 * Struct which holds a thread capability which belong to the client
 */
struct Rtcr::Thread_info : Genode::List<Thread_info>::Element
{
	/**
	 * Capability of the thread
	 */
	Genode::Thread_capability thread_cap;
	Genode::String<Genode::Cpu_session::THREAD_NAME_LEN> name;

	/**
	 * Constructor
	 */
	Thread_info(Genode::Thread_capability thread_cap)
	:
		thread_cap(thread_cap)
	{ }

	Thread_info *find_by_cap(Genode::Thread_capability cap)
	{
		if(thread_cap == cap)
			return this;
		Thread_info *thread_info = next();
		return thread_info ? thread_info->find_by_cap(cap) : 0;
	}

};

#endif /* _RTCR_THREAD_INFO_COMPONENT_H_ */
