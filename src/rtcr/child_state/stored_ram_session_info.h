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
#include "../intercept/ram_session_component.h"
#include "../util/ref_badge.h"

namespace Rtcr {
	struct Stored_ram_session_info;
}


struct Rtcr::Stored_ram_session_info : Genode::List<Stored_ram_session_info>::Element
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
	Genode::List<Ref_badge> ref_badge_infos;

	Stored_ram_session_info()
	:
		kcap(0), badge(0), args(""), ref_badge_infos()
	{ }

	Stored_ram_session_info(Ram_session_component &comp)
	:
		kcap(0), badge(comp.cap().local_name()), args(""),
		ref_badge_infos()
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

		Genode::print(output, "<", Hex(kcap), ", ", badge, "> args=", args);
	}

};

#endif /* _RTCR_STORED_RAM_SESSION_INFO_H_ */
