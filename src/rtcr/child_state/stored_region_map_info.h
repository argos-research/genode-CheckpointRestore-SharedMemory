/*
 * \brief  Structure for storing Region map information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_REGION_MAP_INFO_H_
#define _RTCR_STORED_REGION_MAP_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "stored_info_structs.h"
#include "stored_attached_region_info.h"
#include "../intercept/region_map_component.h"

namespace Rtcr {
	struct Stored_region_map_info;
}


struct Rtcr::Stored_region_map_info : Stored_normal_info, Genode::List<Stored_region_map_info>::Element
{
	Genode::size_t   const size;
	Genode::uint16_t const ds_badge;
	Genode::uint16_t sigh_badge;
	Genode::List<Stored_attached_region_info> stored_attached_region_infos;


	Stored_region_map_info(Region_map_component &region_map, Genode::addr_t targets_kcap)
	:
		Stored_normal_info(targets_kcap,
				region_map.cap().local_name(),
				region_map.parent_state().bootstrapped),
		size(region_map.parent_state().size),
		ds_badge(region_map.parent_state().ds_cap.local_name()),
		sigh_badge(region_map.parent_state().sigh.local_name()),
		stored_attached_region_infos()
	{ }

	Stored_region_map_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_region_map_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_normal_info::print(output);
		Genode::print(output, ", size=", size, ", ds_badge=", ds_badge, ", sigh_badge=", sigh_badge);
	}

};

#endif /* _RTCR_STORED_REGION_MAP_INFO_H_ */
