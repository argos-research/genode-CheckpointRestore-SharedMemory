/*
 * \brief  Monitoring signal context creation
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_SIGNAL_CONTEXT_INFO_COMPONENT_H_
#define _RTCR_SIGNAL_CONTEXT_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <base/signal.h>

namespace Rtcr {
	struct Signal_context_info;
}

/**
 * List element to store Signal_context_capabilities created by the pd session
 */
struct Rtcr::Signal_context_info : Genode::List<Signal_context_info>::Element
{
	Genode::Signal_context_capability          sc_cap;
	Genode::Capability<Genode::Signal_source>  ss_cap;
	/**
	 * imprint is an opaque number (Source: pd_session/pd_session.h),
	 * which is associated with the pointer of a Signal_context in Signal_receiver::manage
	 *
	 * It is sent with each signal.
	 * The usage of imprint is also opaque. It could be used as an process-unique identifier.
	 * The pointer is valid in the process which uses this virtual pd session.
	 *
	 * This means, when restoring the address space and this imprint value. It shall eventually
	 * point to the Signal context used to create this Signal_context_capability
	 */
	unsigned long                              imprint;

	Signal_context_info(Genode::Signal_context_capability sc_cap,
			Genode::Capability<Genode::Signal_source> ss_cap, unsigned long imprint)
	:
		sc_cap(sc_cap),
		ss_cap(ss_cap),
		imprint(imprint)
	{ }

	Signal_context_info *find_by_sc_cap(Genode::Signal_context_capability cap)
	{
		if(cap == sc_cap)
			return this;
		Signal_context_info *info = next();
		return info ? info->find_by_sc_cap(cap) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "sc ", sc_cap, ", ss ", ss_cap, ", imprint=", Hex(imprint));
	}
};


#endif /* _RTCR_SIGNAL_CONTEXT_INFO_COMPONENT_H_ */
