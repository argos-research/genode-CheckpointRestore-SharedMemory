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
#include "../monitor/region_map_info.h"
#include "../intercept/region_map_component.h"
//#include "stored_attached_region_info.h"

namespace Rtcr {
	struct Stored_region_map_info;

	// Forward declaration
	struct Stored_attached_region_info;
}


struct Rtcr::Stored_region_map_info : Genode::List<Stored_region_map_info>::Element
{
	/**
	 * Child's kcap (kernel capability selector)
	 */
	Genode::addr_t   kcap;
	/**
	 * Genode's system-global capability identifier
	 */
	Genode::uint16_t badge;
	Genode::size_t   size;
	Genode::uint16_t fault_handler_badge;
	Genode::uint16_t ds_badge;
	Genode::List<Stored_attached_region_info> stored_attached_region_infos;

	Stored_region_map_info()
	:
		kcap(0), badge(0), size(0), fault_handler_badge(0), ds_badge(0),
		stored_attached_region_infos()
	{ }

	Stored_region_map_info(Region_map_info &info)
	:
		kcap(0), badge(info.region_map.cap().local_name()), size(info.size),
		fault_handler_badge(info.region_map.parent_state().fault_handler.local_name()),
		ds_badge(info.ds_cap.local_name()), stored_attached_region_infos()
	{ }

	Stored_region_map_info(Region_map_component &comp)
	:
		kcap(0), badge(comp.cap().local_name()), size(0),
		fault_handler_badge(comp.parent_state().fault_handler.local_name()),
		ds_badge(0), stored_attached_region_infos()
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

		Genode::print(output, "<", Hex(kcap), ", ", badge, "> size=", Hex(size),
				" fault_handler_badge=", fault_handler_badge, " ds_badge=", ds_badge);
	}

};

#endif /* _RTCR_STORED_REGION_MAP_INFO_H_ */
