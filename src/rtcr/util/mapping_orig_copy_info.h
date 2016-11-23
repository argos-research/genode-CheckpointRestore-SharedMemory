/*
 * \brief  Mapping original dataspace to copy dataspace
 * \author Denis Huber
 * \date   2016-11-23
 */

#ifndef _RTCR_MAPPING_ORIG_COPY_INFO_H_
#define _RTCR_MAPPING_ORIG_COPY_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Mapping_orig_copy_info;
}


struct Rtcr::Mapping_orig_copy_info : Genode::List<Mapping_orig_copy_info>::Element
{
	Genode::Dataspace_capability     const orig_ds_cap;
	Genode::Ram_dataspace_capability const copy_ds_cap;
	Genode::addr_t const copy_rel_addr;
	Genode::size_t const copy_size;

	Mapping_orig_copy_info(Genode::Dataspace_capability orig_ds_cap, Genode::Ram_dataspace_capability copy_ds_cap,
			Genode::addr_t copy_rel_addr, Genode::size_t copy_size)
	:
		orig_ds_cap(orig_ds_cap), copy_ds_cap(copy_ds_cap),
		copy_rel_addr(copy_rel_addr), copy_size(copy_size)
	{ }

	Mapping_orig_copy_info *find_by_orig_badge(Genode::uint16_t badge)
	{
		if(badge == orig_ds_cap.local_name())
			return this;
		Mapping_orig_copy_info *info = next();
		return info ? info->find_by_orig_badge(badge) : 0;
	}

	Mapping_orig_copy_info *find_by_copy_badge(Genode::uint16_t badge)
	{
		if(badge == copy_ds_cap.local_name())
			return this;
		Mapping_orig_copy_info *info = next();
		return info ? info->find_by_copy_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "orig ", orig_ds_cap, ", copy ", copy_ds_cap,
				", copy_addr=", copy_rel_addr, ", copy_size=", copy_size);
	}
};


#endif /* _RTCR_MAPPING_ORIG_COPY_INFO_H_ */
