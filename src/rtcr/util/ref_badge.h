/*
 * \brief  Structure for storing badges in a list
 * \author Denis Huber
 * \date   2016-11-18
 */

#ifndef _RTCR_REF_BADGE_H_
#define _RTCR_REF_BADGE_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */


namespace Rtcr {
	struct Ref_badge;
}


struct Rtcr::Ref_badge : Genode::List<Ref_badge>::Element
{
	Genode::uint16_t ref_badge;

	Ref_badge() : ref_badge(0) { }
	Ref_badge(Genode::uint16_t badge) : ref_badge(badge) { }

	Ref_badge *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->ref_badge)
			return this;
		Ref_badge *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<ref_badge=", ref_badge, ">");
	}
};

#endif /* _RTCR_REF_BADGE_H_ */
