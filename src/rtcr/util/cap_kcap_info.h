/*
 * \brief  List element to store a mapping of Genode's capability to capability space slot (kcap)
 * \author Denis Huber
 * \date   2016-11-30
 */

#ifndef _RTCR_CAP_KCAP_INFO_H_
#define _RTCR_CAP_KCAP_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Cap_kcap_info;
}

/**
 * Structure to store cap-kcap tuple
 */
struct Rtcr::Cap_kcap_info : Genode::List<Cap_kcap_info>::Element
{
	Genode::addr_t            kcap;
	Genode::Native_capability cap;

	Cap_kcap_info(Genode::addr_t kcap, Genode::Native_capability cap)
	: kcap(kcap), cap(cap) { }

	Cap_kcap_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->cap.local_name())
			return this;
		Cap_kcap_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Cap_kcap_info *find_by_kcap(Genode::addr_t kcap)
	{
		if(kcap == this->kcap)
			return this;
		Cap_kcap_info *info = next();
		return info ? info->find_by_kcap(kcap) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "kcap=", Hex(kcap), ", cap=", cap);
	}
};

#endif /* _RTCR_CAP_KCAP_INFO_H_ */
