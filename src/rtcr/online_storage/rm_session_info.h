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
#include "../intercept/region_map_component.h"
#include "../online_storage/info_structs.h"

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
	Genode::Lock region_maps_lock;
    /**
     * List for monitoring Rpc object
     */
	Genode::List<Region_map_component> region_maps;

	Rm_session_info(const char* creation_args, bool bootstrapped)
	:
		Session_rpc_info(creation_args, "", bootstrapped),
		region_maps_lock(), region_maps()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_RM_SESSION_INFO_H_ */
