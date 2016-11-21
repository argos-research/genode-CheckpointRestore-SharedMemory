/*
 * \brief  Stores RM session state
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_RM_SESSION_INFO_H_
#define _RTCR_RM_SESSION_INFO_H_

/* Genode includes */

/* Rtcr includes */
#include "info_structs.h"

namespace Rtcr {
	struct Rm_session_info;
}

/**
 * State information about an RM session
 */
struct Rtcr::Rm_session_info : Session_rpc_info
{
	Rm_session_info(const char* creation_args, bool bootstrapped = false)
	:
		Session_rpc_info(creation_args, "", bootstrapped)
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_RM_SESSION_INFO_H_ */
