/*
 * \brief  Stores RAM session state
 * \author Denis Huber
 * \date   2016-11-21
 */

#ifndef _RTCR_RAM_SESSION_INFO_H_
#define _RTCR_RAM_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "info_structs.h"
#include "ram_dataspace_info.h"

namespace Rtcr {
	struct Ram_session_info;
}

/**
 * State information about a RAM session
 */
struct Rtcr::Ram_session_info : Session_rpc_info
{
	Genode::Ram_session_capability   ref_account_cap;
	/**
	 * Objects lock
	 */
	Genode::Lock                     objs_lock;
	/**
	 * List of allocated ram dataspaces
	 */
	Genode::List<Ram_dataspace_info> normal_objs;

	Ram_session_info(const char* creation_args, bool bootstrapped = false)
	:
		Session_rpc_info(creation_args, "", bootstrapped),
		ref_account_cap(), objs_lock(), normal_objs()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_RAM_SESSION_INFO_H_ */
