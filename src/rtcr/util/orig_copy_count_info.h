/*
 * \brief  List element to store a mapping of badge to dataspace
 * \author Denis Huber
 * \date   2016-11-24
 */

#ifndef _RTCR_ORIG_BADGE_INFO_H_
#define _RTCR_ORIG_BADGE_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Orig_badge_info;
}

/**
 * Structure to store badge-kcap tuple from a capability map of a Target_child
 */
struct Rtcr::Orig_badge_info : Genode::List<Orig_badge_info>::Element
{
	Genode::uint16_t orig_badge;
	Genode::Ram_dataspace_capability copy_dataspace;
	unsigned ref_count;

	Orig_badge_info(Genode::uint16_t badge, Genode::Ram_dataspace_capability dataspace)
	: orig_badge(badge), copy_dataspace(dataspace), ref_count(1) { }

	Orig_badge_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->orig_badge)
			return this;
		Orig_badge_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "badge=", orig_badge, ", copy_dataspace ", copy_dataspace);
	}
};

#endif /* _RTCR_ORIG_BADGE_INFO_H_ */
