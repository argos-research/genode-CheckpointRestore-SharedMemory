/*
 * \brief  Structure for storing Region_map attachments
 * \author Denis Huber
 * \date   2016-10-25
 */

#ifndef _RTCR_STORED_REGION_INFO_H_
#define _RTCR_STORED_REGION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/attached_region_info.h"


namespace Rtcr {
	struct Stored_attached_region_info;
}

struct Rtcr::Stored_attached_region_info : Genode::List<Stored_attached_region_info>::Element
{
	/**
	 * Identifier of the stored dataspace
	 */
	Genode::uint16_t badge;
	Genode::size_t   size;
	Genode::off_t    offset;
	Genode::addr_t   rel_addr;
	bool             executable;

	Stored_attached_region_info()
	:
		badge(0), size(0), offset(0), rel_addr(0), executable(false)
	{ }

	Stored_attached_region_info(Attached_region_info &info)
	:
		badge      (info.ds_cap.local_name()),
		size       (info.size),
		offset     (info.offset),
		rel_addr   (info.rel_addr),
		executable (info.executable)
	{ }

	Stored_attached_region_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_attached_region_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Stored_attached_region_info *find_by_badge_and_addr(Genode::uint16_t badge, Genode::addr_t addr)
	{
		if(badge == this->badge && addr == this->rel_addr)
			return this;
		Stored_attached_region_info *info = next();
		return info ? info->find_by_badge_and_addr(badge, addr) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<ref_badge=", badge, ">",
				" [", Hex(rel_addr, Hex::PREFIX, Hex::PAD),
				", ", Hex(rel_addr + size, Hex::PREFIX, Hex::PAD), ")",
				executable?" exec":"");
	}

};

#endif /* _RTCR_STORED_REGION_INFO_COMPONENT_H_ */
