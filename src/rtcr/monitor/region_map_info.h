/*
 * \brief  Monitoring region map creation/destruction
 * \author Denis Huber
 * \date   2016-10-07
 */

#ifndef _RTCR_REGION_MAP_INFO_COMPONENT_H_
#define _RTCR_REGION_MAP_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
//#include "../intercept/rm_session.h"

namespace Rtcr {
	struct Region_map_info;

	// Forward declaration
	struct Region_map_component;
}

/**
 * List element for managing Region_map_components created through an Rm_session
 *
 * Provides information about Region_map capability, and attached regions
 */
struct Rtcr::Region_map_info : Genode::List<Region_map_info>::Element
{
	/**
	 * Reference to session object; encapsulates capability and object's state
	 */
	Region_map_component &region_map;

	Region_map_info(Region_map_component &region_map)
	:
		region_map(region_map)
	{ }

	Region_map_info *find_by_cap(Genode::Capability<Genode::Region_map> cap)
	{
		if(cap == region_map.cap())
			return this;
		Region_map_info *rm_info = next();
		return rm_info ? rm_info->find_by_cap(cap) : 0;
	}
};

#endif /* _RTCR_REGION_MAP_INFO_COMPONENT_H_ */
