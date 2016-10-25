/*
 * \brief  Structure for storing Region_map attachments
 * \author Denis Huber
 * \date   2016-10-25
 */

#ifndef _RTCR_STORED_REGION_INFO_COMPONENT_H_
#define _RTCR_STORED_REGION_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <dataspace/capability.h>


namespace Rtcr {
	struct Stored_region_info;
}

struct Rtcr::Stored_region_info : Genode::List<Stored_region_info>::Element
{
	Genode::Dataspace_capability ds_cap;
	Genode::addr_t               rel_addr;
	Genode::size_t               size;
	Genode::off_t                offset;
	bool                         managed;

	Stored_region_info(Genode::Dataspace_capability ds_cap,
			Genode::addr_t rel_addr,
			Genode::size_t size,
			Genode::off_t offset,
			bool managed)
	:
		ds_cap   (ds_cap),
		rel_addr (rel_addr),
		size     (size),
		offset   (offset),
		managed  (managed)
	{ }

	Stored_region_info *find_by_ds_cap(Genode::Dataspace_capability cap)
	{
		if(cap == ds_cap)
			return this;
		Stored_region_info *info = next();
		return info ? info->find_by_ds_cap(cap) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, ds_cap);
		Genode::print(output, " [");
		Genode::print(output, Hex(rel_addr, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ", ");
		Genode::print(output, Hex(rel_addr + size, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ")");
	}

};

#endif /* _RTCR_STORED_REGION_INFO_COMPONENT_H_ */
