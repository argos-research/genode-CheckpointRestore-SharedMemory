/*
 * \brief  Stores Region map state
 * \author Denis Huber
 * \date   2016-10-07
 */

#ifndef _RTCR_REGION_MAP_INFO_H_
#define _RTCR_REGION_MAP_INFO_H_

/* Genode includes */

/* Rtcr includes */
#include "info_structs.h"


namespace Rtcr {
	struct Region_map_info;
}

/**
 * List element for managing Region_map_components created through an Rm_session
 */
struct Rtcr::Region_map_info : Normal_rpc_info
{
	/**
	 * Size of the region map
	 */
	const Genode::size_t size;
	/**
	 * Dataspace representation
	 */
	const Genode::Dataspace_capability ds_cap;

	Region_map_info(Genode::size_t size, Genode::Dataspace_capability ds_cap, bool bootstrapped = false)
	:
		Normal_rpc_info(bootstrapped),
		size(size), ds_cap(ds_cap)
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "size=", Hex(size), " ds ", ds_cap, ", ");
		Normal_rpc_info::print(output);
	}
};

#endif /* _RTCR_REGION_MAP_INFO_H_ */
