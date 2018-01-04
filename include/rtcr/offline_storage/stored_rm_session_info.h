/*
 * \brief  Structure for storing RM session information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_RM_SESSION_INFO_H_
#define _RTCR_STORED_RM_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../intercept/rm_session.h"
#include "../offline_storage/stored_info_structs.h"
#include "../offline_storage/stored_region_map_info.h"

namespace Rtcr {
	struct Stored_rm_session_info;
}


struct Rtcr::Stored_rm_session_info : Stored_session_info, Genode::List<Stored_rm_session_info>::Element
{
	Genode::List<Stored_region_map_info> stored_region_map_infos;

	Stored_rm_session_info(Rm_session_component &rm_session, Genode::addr_t targets_kcap)
	:
		Stored_session_info(rm_session.parent_state().creation_args.string(),
				rm_session.parent_state().upgrade_args.string(),
				targets_kcap,
				rm_session.cap().local_name(),
				rm_session.parent_state().bootstrapped),
		stored_region_map_infos()
	{ }

	Stored_rm_session_info(const char* creation_args,
                                        const char* upgrade_args,
                                        Genode::addr_t kcap,
                                        Genode::uint16_t local_name,
                                        bool bootstrapped)
	:
		Stored_session_info(creation_args,upgrade_args,kcap,local_name,bootstrapped),
		stored_region_map_infos()
	{ }

	Stored_rm_session_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_rm_session_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_session_info::print(output);
	}

};

#endif /* _RTCR_STORED_RM_SESSION_INFO_H_ */
