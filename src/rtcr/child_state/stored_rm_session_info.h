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
#include "../monitor/rm_session_info.h"

namespace Rtcr {
	struct Stored_rm_session_info;

	// Forward declaration
	struct Stored_region_map_info;
}


struct Rtcr::Stored_rm_session_info : Genode::List<Stored_rm_session_info>::Element
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
	Genode::List<Stored_region_map_info> stored_region_map_infos;

	Stored_rm_session_info()
	:
		kcap(0), badge(0), args(""), stored_region_map_infos()
	{ }

	Stored_rm_session_info(Rm_session_info &info)
	:
		kcap(0), badge(info.session.cap().local_name()), args(info.args),
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

		Genode::print(output, "<", Hex(kcap), ", ", badge, "> args=", args);
	}

};

#endif /* _RTCR_STORED_RM_SESSION_INFO_H_ */
