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

	Region_map_info(Region_map_component &region_map, Genode::size_t size)
	:
		region_map(region_map), size(size)
	{ }

	Region_map_info *find_by_cap(Genode::Capability<Genode::Region_map> cap)
	{
		if(cap == region_map.cap())
			return this;
		Region_map_info *info = next();
		return info ? info->find_by_cap(cap) : 0;
	}
};

#endif /* _RTCR_REGION_MAP_INFO_H_ */
