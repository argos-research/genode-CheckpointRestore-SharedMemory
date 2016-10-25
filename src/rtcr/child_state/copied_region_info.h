/*
 * \brief  Structure for copying Region_map attachments
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_COPIED_REGION_INFO_COMPONENT_H_
#define _RTCR_COPIED_REGION_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <dataspace/capability.h>

/* Rtcr includes */
#include "../monitor/attached_region_info.h"

namespace Rtcr {
	struct Copied_region_info;
}

struct Rtcr::Copied_region_info : Genode::List<Copied_region_info>::Element
{
	Genode::Dataspace_capability orig_ds_cap;
	Genode::Dataspace_capability copy_ds_cap;
	Genode::addr_t               rel_addr;
	Genode::size_t               size;
	Genode::off_t                offset;
	bool                         managed;

	Copied_region_info(Genode::Dataspace_capability original,
			Genode::Dataspace_capability copy,
			Genode::addr_t rel_addr,
			Genode::size_t size,
			Genode::off_t offset,
			bool managed)
	:
		orig_ds_cap (original),
		copy_ds_cap (copy),
		rel_addr    (rel_addr),
		size        (size),
		offset      (offset),
		managed     (managed)
	{ }
	Copied_region_info(Attached_region_info &orig_info,
			Genode::Dataspace_capability copy_ds_cap,
			bool managed)
	:
		orig_ds_cap (orig_info.ds_cap),
		copy_ds_cap (copy_ds_cap),
		rel_addr    (orig_info.rel_addr),
		size        (orig_info.size),
		offset      (orig_info.offset),
		managed     (managed)
	{ }
	Copied_region_info *find_by_orig_ds_cap(Genode::Dataspace_capability original)
	{
		if(original == orig_ds_cap)
			return this;
		Copied_region_info *info = next();
		return info ? info->find_by_orig_ds_cap(original) : 0;
	}
	Copied_region_info *find_by_copy_ds_cap(Genode::Dataspace_capability copy)
	{
		if(copy == copy_ds_cap)
			return this;
		Copied_region_info *info = next();
		return info ? info->find_by_copy_ds_cap(copy) : 0;
	}
	/**
	 * Find Copied_region_info with the same orig_ds_cap and the same address (e.g. corresponding Attached_region_info)
	 */
	Copied_region_info *find_by_orig_cap_and_addr(Genode::Dataspace_capability cap, Genode::addr_t addr)
	{
		if(cap == orig_ds_cap && addr == rel_addr)
			return this;
		Copied_region_info *info = next();
		return info ? info->find_by_orig_cap_and_addr(cap, addr) : 0;
	}
	/**
	 * Find Copied_region_info with the same copy_ds_cap, but another address (e.g. shared ds_cap)
	 */
	Copied_region_info *find_by_copy_cap_and_not_addr(Genode::Dataspace_capability cap, Genode::addr_t addr)
	{
		if(cap == copy_ds_cap && addr != rel_addr)
			return this;
		Copied_region_info *info = next();
		return info ? info->find_by_copy_cap_and_not_addr(cap, addr) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, orig_ds_cap);
		Genode::print(output, " -> ");
		Genode::print(output, copy_ds_cap);
		Genode::print(output, " [");
		Genode::print(output, Hex(rel_addr, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ", ");
		Genode::print(output, Hex(rel_addr + size, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ")");
	}

};

#endif /* _RTCR_COPIED_REGION_INFO_COMPONENT_H_ */
