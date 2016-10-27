/*
 * \brief  Checkpointer of Target_state
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_CHECKPOINTER_H_
#define _RTCR_CHECKPOINTER_H_

/* Genode includes */
#include <util/list.h>
#include <region_map/client.h>

/* Rtcr includes */
#include "target_state.h"
#include "target_child.h"
#include "intercept/rm_session.h"
#include "intercept/region_map_component.h"
#include "intercept/pd_session_component.h"
#include "intercept/cpu_session_component.h"
#include "intercept/ram_session_component.h"
#include "intercept/rm_session.h"
#include "intercept/log_session.h"
#include "intercept/timer_session.h"

namespace Rtcr {
	class Checkpointer;
	struct Capability_map_info;

	constexpr bool checkpointer_verbose_debug = true;
}


struct Rtcr::Capability_map_info : Genode::List<Capability_map_info>::Element
{
	Genode::addr_t   kcap;
	Genode::uint16_t badge;

	Capability_map_info(Genode::addr_t kcap, Genode::uint16_t badge)
	: kcap(kcap), badge(badge) { }

	Capability_map_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Capability_map_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Capability_map_info *find_by_kcap(Genode::addr_t kcap)
	{
		if(kcap == this->kcap)
			return this;
		Capability_map_info *info = next();
		return info ? info->find_by_kcap(kcap) : 0;
	}
};


class Rtcr::Checkpointer
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = checkpointer_verbose_debug;
	Target_child &_child;
	Target_state &_state;
	Genode::List<Capability_map_info> _capability_map_infos;

	Genode::List<Stored_rm_session_info>      _checkpoint_state(Rm_root &rm_root);
	Genode::List<Stored_region_map_info>      _checkpoint_state(Rm_session_component &rm_session_comp);
	Genode::List<Stored_attached_region_info> _checkpoint_state(Region_map_component &region_map_comp);
	Genode::List<Stored_log_session_info>     _checkpoint_state(Log_root &log_root);
	Genode::List<Stored_timer_session_info>   _checkpoint_state(Timer_root &timer_root);
	Genode::List<Stored_thread_info>          _checkpoint_state(Cpu_session_component &cpu_session_comp);
	Genode::List<Stored_signal_context_info>  _checkpoint_state(Pd_session_component &pd_session_comp);
	Genode::List<Stored_signal_source_info>   _checkpoint_state(Pd_session_component &pd_session_comp);

	Genode::List<Stored_dataspace_info> _checkpoint_dataspaces(Ram_session_component &ram_session_comp,
			Genode::List<Attached_region_info> &address_space);

	Stored_region_map_info _checkpoint_region_map(Region_map_component &region_map_comp);


public:
	Checkpointer(Target_child &child, Target_state &state);

	void checkpoint();
};

#endif /* _RTCR_CHECKPOINTER_H_ */
