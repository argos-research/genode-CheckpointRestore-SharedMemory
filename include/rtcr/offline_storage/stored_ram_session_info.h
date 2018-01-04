/*
 * \brief  Structure for storing RAM session information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_RAM_SESSION_INFO_H_
#define _RTCR_STORED_RAM_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../intercept/ram_session.h"
#include "../offline_storage/stored_info_structs.h"
#include "../offline_storage/stored_ram_dataspace_info.h"

namespace Rtcr {
	struct Stored_ram_session_info;
}


struct Rtcr::Stored_ram_session_info : Stored_session_info, Genode::List<Stored_ram_session_info>::Element
{
	Genode::List<Stored_ram_dataspace_info> stored_ramds_infos;

	Stored_ram_session_info(Ram_session_component &ram_session, Genode::addr_t targets_kcap)
	:
		Stored_session_info(ram_session.parent_state().creation_args.string(),
				ram_session.parent_state().upgrade_args.string(),
				targets_kcap,
				ram_session.cap().local_name(),
				ram_session.parent_state().bootstrapped),
		stored_ramds_infos()
	{ }

	Stored_ram_session_info(const char* creation_args,
                                        const char* upgrade_args,
                                        Genode::addr_t kcap,
                                        Genode::uint16_t local_name,
                                        bool bootstrapped)
	:
		Stored_session_info(creation_args,upgrade_args,kcap,local_name,bootstrapped),
		stored_ramds_infos()
	{ }

	Stored_ram_session_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_ram_session_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_session_info::print(output);
	}

};

#endif /* _RTCR_STORED_RAM_SESSION_INFO_H_ */
