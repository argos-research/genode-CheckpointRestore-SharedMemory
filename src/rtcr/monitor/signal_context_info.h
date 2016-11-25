/*
 * \brief  Monitoring PD::alloc_context and PD::free_context
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_SIGNAL_CONTEXT_INFO_H_
#define _RTCR_SIGNAL_CONTEXT_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <base/signal.h>

/* Rtcr includes */
#include "info_structs.h"

namespace Rtcr {
	struct Signal_context_info;
}

/**
 * List element to store Signal_context_capabilities created by the pd session
 */
struct Rtcr::Signal_context_info : Normal_obj_info, Genode::List<Signal_context_info>::Element
{
	const Genode::Signal_context_capability          sc_cap;
	const Genode::Capability<Genode::Signal_source>  ss_cap;
	const unsigned long imprint;

	Signal_context_info(Genode::Signal_context_capability sc_cap,
			Genode::Capability<Genode::Signal_source> ss_cap, unsigned long imprint, bool bootstrapped)
	:
		Normal_obj_info(bootstrapped),
		sc_cap(sc_cap),
		ss_cap(ss_cap),
		imprint(imprint)
	{ }

	Signal_context_info *find_by_sc_badge(Genode::uint16_t badge)
	{
		if(badge == sc_cap.local_name())
			return this;
		Signal_context_info *info = next();
		return info ? info->find_by_sc_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "sc ", sc_cap, ", ss ", ss_cap, ", imprint=", Hex(imprint), ", ");
		Normal_obj_info::print(output);
	}
};


#endif /* _RTCR_SIGNAL_CONTEXT_INFO_H_ */
