/*
 * \brief  List element to store the designated kcap address for a given badge
 * \author Denis Huber
 * \date   2016-02-20
 */

#ifndef _RTCR_KCAP_BADGE_INFO_H_
#define _RTCR_KCAP_BADGE_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Kcap_badge_info;
}

/**
 * List element to store the designated kcap address for a given badge
 */
struct Rtcr::Kcap_badge_info : Genode::List<Kcap_badge_info>::Element
{
	Genode::addr_t   kcap;
	Genode::uint16_t badge;

	Kcap_badge_info(Genode::addr_t kcap, Genode::uint16_t badge)
	: kcap(kcap), badge(badge) { }

	Kcap_badge_info *find_by_kcap(Genode::addr_t kcap)
	{
		if(kcap == this->kcap)
			return this;
		Kcap_badge_info *info = next();
		return info ? info->find_by_kcap(kcap) : 0;
	}

	Kcap_badge_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Kcap_badge_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "kcap=", Hex(kcap), ", badge=", badge);
	}
};

#endif /* _RTCR_KCAP_BADGE_INFO_H_ */
