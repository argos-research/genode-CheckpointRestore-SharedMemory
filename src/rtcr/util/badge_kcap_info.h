/*
 * \brief  List element to store a mapping of kcap to badge
 * \author Denis Huber
 * \date   2016-11-24
 */

#ifndef _RTCR_BADGE_KCAP_INFO_H_
#define _RTCR_BADGE_KCAP_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Badge_kcap_info;
}

/**
 * Structure to store badge-kcap tuple from a capability map of a Target_child
 */
struct Rtcr::Badge_kcap_info : Genode::List<Badge_kcap_info>::Element
{
	Genode::addr_t   kcap;
	Genode::uint16_t badge;

	Badge_kcap_info(Genode::addr_t kcap, Genode::uint16_t badge)
	: kcap(kcap), badge(badge) { }

	Badge_kcap_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Badge_kcap_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Badge_kcap_info *find_by_kcap(Genode::addr_t kcap)
	{
		if(kcap == this->kcap)
			return this;
		Badge_kcap_info *info = next();
		return info ? info->find_by_kcap(kcap) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "kcap=", Hex(kcap), ", badge=", badge);
	}
};

#endif /* _RTCR_BADGE_KCAP_INFO_H_ */
