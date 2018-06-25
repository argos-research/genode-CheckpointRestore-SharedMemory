/*
 * \brief  List element to associate a stored badge to a new capability (e.g. checkpointed badge
 *         to a newly created capability)
 * \author Denis Huber
 * \date   2016-02-20
 */

#ifndef _RTCR_BADGE_TRANSLATION_INFO_H_
#define _RTCR_BADGE_TRANSLATION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */


namespace Rtcr {
	struct Badge_translation_info;
}


/**
 * List element to associate a stored badge to a new capability
 */
struct Rtcr::Badge_translation_info : Genode::List<Badge_translation_info>::Element
{
	Genode::uint16_t          ckpt_badge;
	Genode::Native_capability resto_cap;

	Badge_translation_info(Genode::uint16_t ckpt_badge, Genode::Native_capability resto_cap)
	: ckpt_badge(ckpt_badge), resto_cap(resto_cap) { }

	Badge_translation_info *find_by_ckpt_badge(Genode::uint16_t badge)
	{
		if(badge == ckpt_badge)
			return this;
		Badge_translation_info *info = next();
		return info ? info->find_by_ckpt_badge(badge) : 0;
	}
	Badge_translation_info *find_by_resto_badge(Genode::uint16_t badge)
	{
		if(badge == resto_cap.local_name())
			return this;
		Badge_translation_info *info = next();
		return info ? info->find_by_resto_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "ckpt_badge=", Hex(ckpt_badge), ", resto ", resto_cap);
	}
};

#endif /* _RTCR_BADGE_TRANSLATION_INFO_H_ */
