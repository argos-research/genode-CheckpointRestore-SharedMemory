/*
 * \brief  List element to store a badge
 * \author Denis Huber
 * \date   2016-11-18
 */

#ifndef _RTCR_REF_BADGE_INFO_H_
#define _RTCR_REF_BADGE_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */


namespace Rtcr {
	struct Ref_badge_info;
}

/**
 * List element to store a badge
 */
struct Rtcr::Ref_badge_info : Genode::List<Ref_badge_info>::Element
{
	Genode::uint16_t ref_badge;

	Ref_badge_info() : ref_badge(0) { }
	Ref_badge_info(Genode::uint16_t badge) : ref_badge(badge) { }

	Ref_badge_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->ref_badge)
			return this;
		Ref_badge_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<ref_badge=", ref_badge, ">");
	}
};

#endif /* _RTCR_REF_BADGE_INFO_H_ */
