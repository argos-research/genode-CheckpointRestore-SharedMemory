/*
 * \brief  Monitoring region map creation/destruction
 * \author Denis Huber
 * \date   2016-10-07
 */

#ifndef _RTCR_REGION_MAP_INFO_H_
#define _RTCR_REGION_MAP_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../intercept/region_map_component.h"


namespace Rtcr {
	struct Region_map_info;
}

/**
 * List element for managing Region_map_components created through an Rm_session
 */
struct Rtcr::Region_map_info : Genode::List<Region_map_info>::Element
{
	/**
	 * Reference to session object; encapsulates capability and object's state
	 */
	Region_map_component &region_map;
	/**
	 * Size of the region map
	 */
	Genode::size_t        size;
	/**
	 * Dataspace representation
	 */
	Genode::Dataspace_capability ds_cap;

	Region_map_info(Region_map_component &region_map, Genode::size_t size, Genode::Dataspace_capability ds_cap)
	:
		region_map(region_map), size(size), ds_cap(ds_cap)
	{ }

	Region_map_info *find_by_cap(Genode::Capability<Genode::Region_map> cap)
	{
		if(cap == region_map.cap())
			return this;
		Region_map_info *info = next();
		return info ? info->find_by_cap(cap) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "region map ", region_map.cap(), " size=", Hex(size), " ds ", ds_cap);

		Attached_region_info *info = region_map.attached_regions().first();
		while(info)
		{
			Genode::print(output, "  ", *info, "\n");
			info = info->next();
		}
	}
};

#endif /* _RTCR_REGION_MAP_INFO_H_ */
