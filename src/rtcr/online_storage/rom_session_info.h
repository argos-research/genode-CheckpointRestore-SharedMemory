/*
 * \brief  Stores ROM session state
 * \author Denis Huber
 * \date   2016-11-22
 */

#ifndef _RTCR_ROM_SESSION_INFO_H_
#define _RTCR_ROM_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../online_storage/info_structs.h"

namespace Rtcr {
	struct Rom_session_info;
}

/**
 * State information about a ROM session
 */
struct Rtcr::Rom_session_info : Session_rpc_info
{
	Genode::Rom_dataspace_capability  dataspace;
	Genode::size_t                    size;
	Genode::Signal_context_capability sigh;

	Rom_session_info(const char* creation_args, bool bootstrapped)
	:
		Session_rpc_info(creation_args, "", bootstrapped),
		dataspace(), size(0), sigh()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, dataspace, ", size=", size, ", sigh ", sigh, ", ");
		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_ROM_SESSION_INFO_H_ */
