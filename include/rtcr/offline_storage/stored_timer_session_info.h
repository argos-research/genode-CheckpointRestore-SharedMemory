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
#include "../intercept/timer_session.h"
#include "../offline_storage/stored_info_structs.h"

namespace Rtcr {
	struct Stored_timer_session_info;
}


struct Rtcr::Stored_timer_session_info : Stored_session_info, Genode::List<Stored_timer_session_info>::Element
{
	Genode::uint16_t sigh_badge;
	unsigned         timeout;
	bool             periodic;

	Stored_timer_session_info(Timer_session_component &timer_session, Genode::addr_t targets_kcap)
	:
		Stored_session_info(timer_session.parent_state().creation_args.string(),
				timer_session.parent_state().upgrade_args.string(),
				targets_kcap,
				timer_session.cap().local_name(),
				timer_session.parent_state().bootstrapped),
		sigh_badge (timer_session.parent_state().sigh.local_name()),
		timeout    (timer_session.parent_state().timeout),
		periodic   (timer_session.parent_state().periodic)
	{ }

	Stored_timer_session_info(const char* creation_args,
                                        const char* upgrade_args,
                                        Genode::addr_t kcap,
                                        Genode::uint16_t local_name,
                                        bool bootstrapped,
				Genode::uint16_t _sigh_badge,
        			unsigned _timeout,
        			bool _periodic)
	:
		Stored_session_info(creation_args,upgrade_args,kcap,local_name,bootstrapped),
		sigh_badge (_sigh_badge),
                timeout    (_timeout),
                periodic   (_periodic)
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

		Stored_session_info::print(output);
		Genode::print(output, ", sigh_badge=", sigh_badge, ", timeout=", timeout, ", periodic=", periodic);
	}

};

#endif /* _RTCR_STORED_TIMER_SESSION_INFO_H_ */
