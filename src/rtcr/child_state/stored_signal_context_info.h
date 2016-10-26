/*
 * \brief  Structure for storing signal context information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_SIGNAL_CONTEXT_INFO_H_
#define _RTCR_STORED_SIGNAL_CONTEXT_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/signal_context_info.h"

namespace Rtcr {
	struct Stored_signal_context_info;
}


struct Rtcr::Stored_signal_context_info : Genode::List<Stored_signal_context_info>::Element
{
	/**
	 * Child's kcap (kernel capability selector)
	 */
	Genode::addr_t   kcap;
	/**
	 * Genode's system-global capability identifier
	 */
	Genode::uint16_t badge;
	Genode::uint16_t signal_source_badge;
	unsigned long    imprint;

	Stored_signal_context_info()
	:
		kcap(0), badge(0), signal_source_badge(0), imprint(0)
	{ }

	Stored_signal_context_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_signal_context_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<", Hex(kcap), ",", badge, ">",
				" signal_source_badge=", signal_source_badge,
				", imprint=", Hex(imprint));
	}

};

#endif /* _RTCR_STORED_SIGNAL_CONTEXT_INFO_H_ */
