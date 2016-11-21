/*
 * \brief  Stores RM session state
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_RM_SESSION_INFO_H_
#define _RTCR_RM_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "info_structs.h"
#include "../intercept/region_map_component.h"

namespace Rtcr {
	struct Rm_session_info;
}

/**
 * State information about an RM session
 */
struct Rtcr::Rm_session_info : Session_rpc_info
{
    /**
     * Lock for infos list
     */
	Genode::Lock objs_lock;
    /**
     * List for monitoring Rpc object
     */
	Genode::List<Region_map_component> normal_rpc_objs;

	Rm_session_info(const char* creation_args, bool bootstrapped = false)
	:
		Session_rpc_info(creation_args, "", bootstrapped),
		objs_lock(), normal_rpc_objs()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_RM_SESSION_INFO_H_ */
