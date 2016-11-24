/*
 * \brief  Structure for storing PD session information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_ROM_SESSION_INFO_H_
#define _RTCR_STORED_ROM_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "stored_info_structs.h"
#include "../intercept/rom_session.h"

namespace Rtcr {
	struct Stored_rom_session_info;
}


struct Rtcr::Stored_rom_session_info : Stored_session_info, Genode::List<Stored_rom_session_info>::Element
{
	Genode::uint16_t dataspace_badge;
	Genode::uint16_t sigh_badge;

	Stored_rom_session_info(Rom_session_component &rom_session, Genode::addr_t targets_kcap,
			Genode::Ram_dataspace_capability copy_ds_cap)
	:
		Stored_session_info(rom_session.parent_state().creation_args.string(),
				rom_session.parent_state().upgrade_args.string(),
				targets_kcap,
				rom_session.cap().local_name(),
				rom_session.parent_state().bootstrapped),
		dataspace_badge (rom_session.parent_state().dataspace.local_name()),
		sigh_badge      (rom_session.parent_state().sigh.local_name())
	{ }

	Stored_rom_session_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_rom_session_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_session_info::print(output);
		Genode::print(output, ", dataspace_badge=", dataspace_badge,
				", sigh_badge=", sigh_badge, ", copy_ds ", memory_content);
	}

};

#endif /* _RTCR_STORED_ROM_SESSION_INFO_H_ */
