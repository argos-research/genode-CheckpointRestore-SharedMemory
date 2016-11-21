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
	Genode::size_t const size;
	/**
	 * Dataspace representation
	 */
	Genode::Dataspace_capability const ds_cap;
	/**
	 * Signal context of the fault handler
	 */
	Genode::Signal_context_capability sigh_cap;
	/**
	 * Lock
	 */
	Genode::Lock                       objs_lock;
	/**
	 * List of attached regions
	 */
	Genode::List<Attached_region_info> normal_objs;

	Region_map_info(Genode::size_t size, Genode::Dataspace_capability ds_cap, bool bootstrapped = false)
	:
		Normal_rpc_info(bootstrapped),
		size(size), ds_cap(ds_cap), sigh_cap(),
		objs_lock(), normal_objs()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "size=", Hex(size), " ds ", ds_cap, ", ");
		Normal_rpc_info::print(output);
	}
};

#endif /* _RTCR_REGION_MAP_INFO_H_ */
