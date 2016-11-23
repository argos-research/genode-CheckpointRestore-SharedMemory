/*
 * \brief  Structure for storing PD session information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_PD_SESSION_INFO_H_
#define _RTCR_STORED_PD_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "stored_info_structs.h"
#include "stored_signal_context_info.h"
#include "stored_signal_source_info.h"
#include "stored_region_map_info.h"
#include "../intercept/pd_session.h"

namespace Rtcr {
	struct Stored_pd_session_info;
}


struct Rtcr::Stored_pd_session_info : Stored_session_info, Genode::List<Stored_pd_session_info>::Element
{
	Genode::List<Stored_signal_context_info> stored_context_infos;
	Genode::List<Stored_signal_source_info> stored_source_infos;
	//Genode::List<Stored_native_capability_info> stored_native_cap_infos;
	Stored_region_map_info stored_address_space;
	Stored_region_map_info stored_stack_area;
	Stored_region_map_info stored_linker_area;

	Stored_pd_session_info(Pd_session_component &pd_session, Genode::addr_t targets_kcap)
	:
		Stored_session_info(pd_session.parent_state().creation_args.string(),
				pd_session.parent_state().upgrade_args.string(),
				targets_kcap,
				pd_session.cap().local_name(),
				pd_session.parent_state().bootstrapped),
		stored_context_infos(), stored_source_infos(),
		stored_address_space(pd_session.address_space_component()),
		stored_stack_area(pd_session.stack_area_component()),
		stored_linker_area(pd_session.linker_area_component())
	{ }

	Stored_pd_session_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_pd_session_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_session_info::print(output);
	}

};

#endif /* _RTCR_STORED_PD_SESSION_INFO_H_ */
