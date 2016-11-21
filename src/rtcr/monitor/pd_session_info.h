/*
 * \brief  Stores PD session state
 * \author Denis Huber
 * \date   2016-11-21
 */

#ifndef _RTCR_PD_SESSION_INFO_H_
#define _RTCR_PD_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "info_structs.h"
#include "signal_source_info.h"
#include "signal_context_info.h"
#include "native_capability_info.h"

namespace Rtcr {
	struct Pd_session_info;
}

/**
 * State information about a PD session
 */
struct Rtcr::Pd_session_info : Session_rpc_info
{
	/**
	 * Lock for Signal_sources
	 */
	Genode::Lock                         source_lock;
	/**
	 * List for monitoring the creation and destruction of Signal_source_capabilities
	 */
	Genode::List<Signal_source_info>     source_objs;
	/**
	 * Lock for Signal_contexts
	 */
	Genode::Lock                         context_lock;
	/**
	 * List for monitoring the creation and destruction of Signal_context_capabilities
	 */
	Genode::List<Signal_context_info>    context_objs;
	/**
	 * Lock for Native_capabilities
	 */
	Genode::Lock                         native_cap_lock;
	/**
	 * List for monitoring the creation and destruction of Native_capabilities
	 */
	Genode::List<Native_capability_info> native_cap_objs;

	Pd_session_info(const char* creation_args, bool bootstrapped = false)
	:
		Session_rpc_info(creation_args, "", bootstrapped),
		source_lock(), source_objs(),
		context_lock(), context_objs(),
		native_cap_lock(), native_cap_objs()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_PD_SESSION_INFO_H_ */
