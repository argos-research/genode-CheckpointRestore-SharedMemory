/*
 * \brief  Structure for storing LOG session information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_LOG_SESSION_INFO_H_
#define _RTCR_STORED_LOG_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/log_session_info.h"

namespace Rtcr {
	struct Stored_log_session_info;
}


struct Rtcr::Stored_log_session_info : Genode::List<Stored_log_session_info>::Element
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

	Stored_log_session_info()
	:
		kcap(0), badge(0), args("")
	{ }

	Stored_log_session_info(Log_session_info &info)
	:
		kcap(0), badge(info.session.cap().local_name()), args(info.args)
	{ }

	Stored_log_session_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_log_session_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<", Hex(kcap), ", ", badge, "> args=", args);
	}

};

#endif /* _RTCR_STORED_LOG_SESSION_INFO_H_ */
