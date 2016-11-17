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
#include "../intercept/pd_session_component.h"

namespace Rtcr {
	struct Stored_pd_session_info;
}


struct Rtcr::Stored_pd_session_info : Genode::List<Stored_pd_session_info>::Element
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
	Genode::List<Stored_signal_context_info> stored_context_infos;
	Genode::List<Stored_signal_source_info> stored_source_infos;
	Stored_region_map_info stored_address_space;
	Stored_region_map_info stored_stack_area;
	Stored_region_map_info stored_linker_area;

	Stored_pd_session_info()
	:
		kcap(0), badge(0), args(""), stored_context_infos(), stored_source_infos(),
		stored_address_space(), stored_stack_area(), stored_linker_area()
	{ }

	Stored_pd_session_info(Pd_session_component &comp)
	:
		kcap(0), badge(comp.cap().local_name()), args(""),
		stored_context_infos(), stored_source_infos(),
		stored_address_space(comp.address_space_component()),
		stored_stack_area(comp.stack_area_component()),
		stored_linker_area(comp.linker_area_component())
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

		Genode::print(output, "<", Hex(kcap), ", ", badge, "> args=", args);
	}

};

#endif /* _RTCR_STORED_PD_SESSION_INFO_H_ */
