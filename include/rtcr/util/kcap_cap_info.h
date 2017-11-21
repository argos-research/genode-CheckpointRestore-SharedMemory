/*
 * \brief  List element to store the designated kcap address for a given capability
 * \author Denis Huber
 * \date   2016-02-20
 */

#ifndef _RTCR_KCAP_CAP_INFO_H_
#define _RTCR_KCAP_CAP_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */

namespace Rtcr {
	struct Kcap_cap_info;
}

/**
 * List element to store the designated kcap address for a given capability
 */
struct Rtcr::Kcap_cap_info : Genode::List<Kcap_cap_info>::Element
{
	Genode::addr_t            kcap;
	Genode::Native_capability cap;
	Genode::String<32>        const label;

	Kcap_cap_info(Genode::addr_t kcap, Genode::Native_capability cap, const char* label)
	: kcap(kcap), cap(cap), label(label) { }

	Kcap_cap_info *find_by_kcap(Genode::addr_t kcap)
	{
		if(kcap == this->kcap)
			return this;
		Kcap_cap_info *info = next();
		return info ? info->find_by_kcap(kcap) : 0;
	}

	Kcap_cap_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->cap.local_name())
			return this;
		Kcap_cap_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "kcap=", Hex(kcap), ", ", cap, ", ", label.string());
	}
};

#endif /* _RTCR_KCAP_CAP_INFO_H_ */
