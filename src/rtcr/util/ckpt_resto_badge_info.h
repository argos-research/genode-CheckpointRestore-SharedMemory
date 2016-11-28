/*
 * \brief  Structure for associating a badge to another capability (e.g. checkpointed badge and restored cap)
 * \author Denis Huber
 * \date   2016-11-26
 */

#ifndef _RTCR_CKPT_RESTO_BADGE_INFO_H_
#define _RTCR_CKPT_RESTO_BADGE_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */


namespace Rtcr {
	struct Ckpt_resto_badge_info;
}


/**
 * List element which translates a badge from the checkpoint process
 * to a badge from the restoration process
 */
struct Rtcr::Ckpt_resto_badge_info : Genode::List<Ckpt_resto_badge_info>::Element
{
	Genode::uint16_t ckpt_badge;
	Genode::Native_capability resto_cap;

	Ckpt_resto_badge_info(Genode::uint16_t ckpt_badge, Genode::Native_capability resto_cap)
	: ckpt_badge(ckpt_badge), resto_cap(resto_cap) { }

	Ckpt_resto_badge_info *find_by_ckpt_badge(Genode::uint16_t badge)
	{
		if(badge == ckpt_badge)
			return this;
		Ckpt_resto_badge_info *info = next();
		return info ? info->find_by_ckpt_badge(badge) : 0;
	}
	Ckpt_resto_badge_info *find_by_resto_badge(Genode::uint16_t badge)
	{
		if(badge == resto_cap.local_name())
			return this;
		Ckpt_resto_badge_info *info = next();
		return info ? info->find_by_resto_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "ckpt_badge=", ckpt_badge, ", resto ", resto_cap);
	}
};

#endif /* _RTCR_CKPT_RESTO_BADGE_INFO_H_ */
