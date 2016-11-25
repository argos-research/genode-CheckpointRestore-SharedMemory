/*
 * \brief  List element to store a mapping of original dataspace, copy dataspace,
 *         and a count how often original dataspace is referenced
 * \author Denis Huber
 * \date   2016-11-24
 */

#ifndef _RTCR_ORIG_COPY_COUNT_INFO_H_
#define _RTCR_ORIG_COPY_COUNT_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Orig_copy_count_info;
}

/**
 * Structure to store badge-kcap tuple from a capability map of a Target_child
 */
struct Rtcr::Orig_copy_count_info : Genode::List<Orig_copy_count_info>::Element
{
	Genode::Dataspace_capability     orig_ds_cap;
	Genode::Ram_dataspace_capability copy_ds_cap;
	Genode::size_t                   size;
	unsigned ref_count;

	Orig_copy_count_info(Genode::Dataspace_capability orig_ds_cap, Genode::Ram_dataspace_capability copy_ds_cap, Genode::size_t size)
	: orig_ds_cap(orig_ds_cap), copy_ds_cap(copy_ds_cap), size(size), ref_count(1) { }

	Orig_copy_count_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->orig_ds_cap.local_name())
			return this;
		Orig_copy_count_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "orig ds ", orig_ds_cap, ", copy ds ", copy_ds_cap,
				", size=", Hex(size), ", ref_count=", ref_count);
	}
};

#endif /* _RTCR_ORIG_COPY_COUNT_INFO_H_ */
