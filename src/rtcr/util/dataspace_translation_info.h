/*
 * \brief  List element to encapsulate an original dataspace and its copy dataspace
 * \author Denis Huber
 * \date   2017-02-20
 */

#ifndef _RTCR_DATASPACE_TRANSLATION_INFO_H_
#define _RTCR_DATASPACE_TRANSLATION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Dataspace_translation_info;
}

/**
 * Structure to encapsulate an original dataspace and its copy dataspace
 */
struct Rtcr::Dataspace_translation_info : Genode::List<Dataspace_translation_info>::Element
{
	Genode::Dataspace_capability     orig_ds_cap;
	Genode::Ram_dataspace_capability copy_ds_cap;
	Genode::size_t                   size;

	Dataspace_translation_info(Genode::Dataspace_capability orig_ds_cap, Genode::Ram_dataspace_capability copy_ds_cap, Genode::size_t size)
	: orig_ds_cap(orig_ds_cap), copy_ds_cap(copy_ds_cap), size(size) { }

	Dataspace_translation_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->orig_ds_cap.local_name())
			return this;
		Dataspace_translation_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "orig ds ", orig_ds_cap, ", copy ds ", copy_ds_cap,
				", size=", Hex(size));
	}
};

#endif /* _RTCR_DATASPACE_TRANSLATION_INFO_H_ */
