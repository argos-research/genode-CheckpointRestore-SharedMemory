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
#include "../intercept/log_session.h"
#include "../offline_storage/stored_info_structs.h"

namespace Rtcr {
	struct Stored_log_session_info;
}


struct Rtcr::Stored_log_session_info : Stored_session_info, Genode::List<Stored_log_session_info>::Element
{

	Stored_log_session_info(Log_session_component &log_session, Genode::addr_t targets_kcap)
	:
		Stored_session_info(log_session.parent_state().creation_args.string(),
				log_session.parent_state().upgrade_args.string(),
				targets_kcap,
				log_session.cap().local_name(),
				log_session.parent_state().bootstrapped)
	{ }

	Stored_log_session_info(const char* creation_args,
                                        const char* upgrade_args,
                                        Genode::addr_t kcap,
                                        Genode::uint16_t local_name,
                                        bool bootstrapped)
        :
                Stored_session_info(creation_args,upgrade_args,kcap,local_name,bootstrapped)
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

		Stored_session_info::print(output);
	}

};

#endif /* _RTCR_STORED_LOG_SESSION_INFO_H_ */
