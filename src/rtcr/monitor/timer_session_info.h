/*
 * \brief  Stores Timer session state
 * \author Denis Huber
 * \date   2016-10-07
 */

#ifndef _RTCR_TIMER_SESSION_INFO_H_
#define _RTCR_TIMER_SESSION_INFO_H_

/* Genode includes */

/* Rtcr includes */
#include "info_structs.h"

namespace Rtcr {
	struct Timer_session_info;
}

/**
 * State information about a Timer session
 */
struct Rtcr::Timer_session_info : Session_rpc_info
{
	Genode::Signal_context_capability sigh;
	unsigned timeout;
	bool     periodic;

	Timer_session_info(const char* creation_args, bool bootstrapped)
	:
		Session_rpc_info(creation_args, "", bootstrapped),
		sigh(), timeout(0), periodic(false)
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "sigh ", sigh, ", timeout=", timeout, "periodic=", periodic, ", ");
		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_TIMER_SESSION_INFO_H_ */
