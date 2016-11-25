/*
 * \brief  List element to store a mapping of original dataspace, copy dataspace,
 *         and a flag whether the original dataspace was checkpointed to copy dataspace
 * \author Denis Huber
 * \date   2016-11-23
 */

#ifndef _RTCR_ORIG_COPY_CKPT_INFO_H_
#define _RTCR_ORIG_COPY_CKPT_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Orig_copy_ckpt_info;
}


struct Rtcr::Orig_copy_ckpt_info : Genode::List<Orig_copy_ckpt_info>::Element
{
	Genode::Dataspace_capability     const orig_ds_cap;
	Genode::Ram_dataspace_capability const copy_ds_cap;
	Genode::addr_t const copy_rel_addr;
	Genode::size_t const copy_size;
	bool checkpointed;

	Orig_copy_ckpt_info(Genode::Dataspace_capability orig_ds_cap, Genode::Ram_dataspace_capability copy_ds_cap,
			Genode::addr_t copy_rel_addr, Genode::size_t copy_size)
	:
		orig_ds_cap(orig_ds_cap), copy_ds_cap(copy_ds_cap),
		copy_rel_addr(copy_rel_addr), copy_size(copy_size),
		checkpointed(false)
	{ }

	Orig_copy_ckpt_info *find_by_orig_badge(Genode::uint16_t badge)
	{
		if(badge == orig_ds_cap.local_name())
			return this;
		Orig_copy_ckpt_info *info = next();
		return info ? info->find_by_orig_badge(badge) : 0;
	}

	Orig_copy_ckpt_info *find_by_copy_badge(Genode::uint16_t badge)
	{
		if(badge == copy_ds_cap.local_name())
			return this;
		Orig_copy_ckpt_info *info = next();
		return info ? info->find_by_copy_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "orig ", orig_ds_cap, ", copy ", copy_ds_cap,
				", copy_addr=", copy_rel_addr, ", copy_size=", copy_size, ", checkpointed=", checkpointed);
	}
};


#endif /* _RTCR_ORIG_COPY_CKPT_INFO_H_ */
