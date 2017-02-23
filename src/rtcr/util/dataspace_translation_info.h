/*
 * \brief  List element to encapsulate a source dataspace and the destination dataspace
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
 * Structure to encapsulate a source dataspace, which is used to copy from,
 * and the destination dataspace, which is used to copy to
 */
struct Rtcr::Dataspace_translation_info : Genode::List<Dataspace_translation_info>::Element
{
	Genode::Ram_dataspace_capability ckpt_ds_cap;
	Genode::Dataspace_capability     resto_ds_cap;
	Genode::size_t                   size;
	bool                             processed;

	Dataspace_translation_info(Genode::Ram_dataspace_capability ckpt_ds_cap, Genode::Dataspace_capability resto_ds_cap, Genode::size_t size)
	: ckpt_ds_cap(ckpt_ds_cap), resto_ds_cap(resto_ds_cap), size(size), processed(false) { }

	Dataspace_translation_info *find_by_resto_badge(Genode::uint16_t badge)
	{
		if(badge == this->resto_ds_cap.local_name())
			return this;
		Dataspace_translation_info *info = next();
		return info ? info->find_by_resto_badge(badge) : 0;
	}

	Dataspace_translation_info *find_by_ckpt_badge(Genode::uint16_t badge)
	{
		if(badge == this->ckpt_ds_cap.local_name())
			return this;
		Dataspace_translation_info *info = next();
		return info ? info->find_by_ckpt_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "resto ds ", resto_ds_cap, ", dest ds ", ckpt_ds_cap,
				", size=", Hex(size));
	}
};

#endif /* _RTCR_DATASPACE_TRANSLATION_INFO_H_ */
