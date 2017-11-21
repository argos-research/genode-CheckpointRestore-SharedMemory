/*
 * \brief  Stores CPU session state
 * \author Denis Huber
 * \date   2016-11-21
 */

#ifndef _RTCR_CPU_SESSION_INFO_H_
#define _RTCR_CPU_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../intercept/cpu_thread_component.h"
#include "../online_storage/cpu_thread_info.h"
#include "../online_storage/info_structs.h"

namespace Rtcr {
	struct Cpu_session_info;
}

/**
 * State information about a CPU session
 */
struct Rtcr::Cpu_session_info : Session_rpc_info
{
	Genode::Signal_context_capability sigh;
	/**
	 * Lock to make _threads thread-safe
	 */
	Genode::Lock cpu_threads_lock;
	/**
	 * List of client's thread capabilities
	 */
	Genode::List<Cpu_thread_component> cpu_threads;

	Cpu_session_info(const char* creation_args, bool bootstrapped)
	:
		Session_rpc_info(creation_args, "", bootstrapped),
		cpu_threads_lock(), cpu_threads()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "sigh ", sigh, ", ");
		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_CPU_SESSION_INFO_H_ */
