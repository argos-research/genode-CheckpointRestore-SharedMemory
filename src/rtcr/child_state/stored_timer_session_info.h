/*
 * \brief  Structure for storing Timer session information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_TIMER_SESSION_INFO_H_
#define _RTCR_STORED_TIMER_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/timer_session_info.h"

namespace Rtcr {
	struct Stored_timer_session_info;
}


struct Rtcr::Stored_timer_session_info : Genode::List<Stored_timer_session_info>::Element
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
	Genode::uint16_t  sigh_badge;
	unsigned          timeout;
	bool              periodic;

	Stored_timer_session_info()
	:
		kcap(0), badge(0), args(""), sigh_badge(0),
		timeout(0), periodic(false)
	{ }

	Stored_timer_session_info(Timer_session_info &info)
	:
		kcap(0), badge(info.session.cap().local_name()), args(info.args),
		sigh_badge(info.session.parent_state().sigh.local_name()),
		timeout(info.session.parent_state().timeout),
		periodic(info.session.parent_state().periodic)
	{ }

	Stored_timer_session_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_timer_session_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<", Hex(kcap), ",", badge, "> args=", args,
				", sigh_badge=", sigh_badge,
				", timeout=", timeout,
				", periodic=", periodic?"y":"n");
	}

};

#endif /* _RTCR_STORED_TIMER_SESSION_INFO_H_ */
