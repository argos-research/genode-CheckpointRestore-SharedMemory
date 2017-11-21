/*
 * \brief  Monitoring PD::alloc_signal_source and PD::free_signal_source
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_SIGNAL_SOURCE_INFO_H_
#define _RTCR_SIGNAL_SOURCE_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <base/capability.h>

/* Rtcr includes */

namespace Rtcr {
	struct Signal_source_info;
}


/**
 * List element to store Signal_source_capabilities created by the pd session
 */
struct Rtcr::Signal_source_info : Normal_obj_info, Genode::List<Signal_source_info>::Element
{
	// Creation result
	Genode::Capability<Genode::Signal_source> const cap;

	Signal_source_info(Genode::Capability<Genode::Signal_source> cap, bool bootstrapped)
	:
		Normal_obj_info(bootstrapped),
		cap(cap)
	{ }

	Signal_source_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == cap.local_name())
			return this;
		Signal_source_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, cap, ", ");
		Normal_obj_info::print(output);
	}
};

#endif /* _RTCR_SIGNAL_SOURCE_INFO_H_ */
