/*
 * \brief  Structure for storing CPU session information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_CPU_SESSION_INFO_H_
#define _RTCR_STORED_CPU_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../intercept/cpu_session_component.h"

namespace Rtcr {
	struct Stored_cpu_session_info;
}


struct Rtcr::Stored_cpu_session_info : Genode::List<Stored_cpu_session_info>::Element
{
	/**
	 * Child's kcap (kernel capability selector)
	 */
	Genode::addr_t    kcap;
	/**
	 * Genode's system-global capability identifier
	 */
	Genode::uint16_t  badge;
	const char       *args;
	Genode::uint16_t  exception_sigh_badge;
	Genode::List<Stored_thread_info> stored_thread_infos;

	Stored_cpu_session_info()
	:
		kcap(0), badge(0), args(""), exception_sigh_badge(0), stored_thread_infos()
	{ }

	Stored_cpu_session_info(Cpu_session_component &comp)
	:
		kcap(0), badge(comp.cap().local_name()), args(""),
		exception_sigh_badge(comp.parent_state().exception_sigh.local_name()),
		stored_thread_infos()
	{ }

	Stored_cpu_session_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_cpu_session_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<", Hex(kcap), ", ", badge, "> args=", args, " sigh_badge=", exception_sigh_badge);
	}

};

#endif /* _RTCR_STORED_CPU_SESSION_INFO_H_ */
