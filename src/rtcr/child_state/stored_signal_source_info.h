/*
 * \brief  Structure for storing signal context information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_SIGNAL_SOURCE_INFO_H_
#define _RTCR_STORED_SIGNAL_SOURCE_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/signal_source_info.h"

namespace Rtcr {
	struct Stored_signal_source_info;
}


struct Rtcr::Stored_signal_source_info : Genode::List<Stored_signal_source_info>::Element
{
	/**
	 * Child's kcap (kernel capability selector)
	 */
	Genode::addr_t   kcap;
	/**
	 * Genode's system-global capability identifier
	 */
	Genode::uint16_t badge;

	Stored_signal_source_info()
	:
		kcap(0), badge(0)
	{ }

	Stored_signal_source_info(Signal_source_info &info)
	:
		kcap(0), badge(info.cap.local_name())
	{ }

	Stored_signal_source_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_signal_source_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<", Hex(kcap), ", ", badge, ">");
	}

};

#endif /* _RTCR_STORED_SIGNAL_SOURCE_INFO_H_ */
