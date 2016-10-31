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
#include "intercept/region_map_component.h"
#include "intercept/pd_session_component.h"
#include "intercept/cpu_session_component.h"
#include "intercept/ram_session_component.h"
#include "child_state/stored_attached_region_info.h"
#include "child_state/stored_dataspace_info.h"
#include "child_state/stored_log_session_info.h"

namespace Rtcr {
	class Checkpointer;
	struct Badge_kcap_info;

	constexpr bool checkpointer_verbose_debug = true;
}


struct Rtcr::Badge_kcap_info : Genode::List<Badge_kcap_info>::Element
{
	Genode::addr_t   kcap;
	Genode::uint16_t badge;

	Badge_kcap_info(Genode::addr_t kcap, Genode::uint16_t badge)
	: kcap(kcap), badge(badge) { }

	Badge_kcap_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Badge_kcap_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Badge_kcap_info *find_by_kcap(Genode::addr_t kcap)
	{
		if(kcap == this->kcap)
			return this;
		Badge_kcap_info *info = next();
		return info ? info->find_by_kcap(kcap) : 0;
	}
};


class Rtcr::Checkpointer
{
private:
	/**
	 * Structure to remember checkpointed dataspaces by storing their badge,
	 * thus, old dataspaces can be deleted at the end of the checkpoint process
	 */
	struct Badge_info : Genode::List<Badge_info>::Element
	{
		Genode::uint16_t badge;
		Badge_info(Genode::uint16_t badge) : badge(badge) { }

		Badge_info *find_by_badge(Genode::uint16_t badge)
		{
			if(badge == this->badge)
				return this;
			Badge_info *info = next();
			return info ? info->find_by_badge(badge) : 0;
		}
	};

	/**
	 * Associate a valid badge to a valid dataspace capability
	 */
	struct Badge_dataspace_info : Genode::List<Badge_dataspace_info>::Element
	{
		Genode::uint16_t badge;
		Genode::Dataspace_capability ds_cap;
		Badge_dataspace_info(Genode::uint16_t badge, Genode::Dataspace_capability ds_cap)
		: badge(badge), ds_cap(ds_cap) { }

		Badge_dataspace_info *find_by_badge(Genode::uint16_t badge)
		{
			if(badge == this->badge)
				return this;
			Badge_dataspace_info *info = next();
			return info ? info->find_by_badge(badge) : 0;
		}
	};

	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = checkpointer_verbose_debug;
	Genode::Allocator &_alloc;
	Target_child      &_child;
	Target_state      &_state;
	Genode::List<Badge_kcap_info> _capability_map_infos;

	void _prepare_cap_map_infos   (Genode::List<Badge_kcap_info>             &state_infos, Genode::Dataspace_capability ds_cap,
	                               Genode::off_t cap_map_start);
	void _prepare_rm_sessions     (Genode::List<Stored_rm_session_info>      &state_infos, Rm_root               &child_obj);
	void _prepare_log_sessions    (Genode::List<Stored_log_session_info>     &state_infos, Log_root              &child_obj);
	void _prepare_timer_sessions  (Genode::List<Stored_timer_session_info>   &state_infos, Timer_root            &child_obj);
	void _prepare_region_maps     (Genode::List<Stored_region_map_info>      &state_infos, Rm_session_component  &child_obj);
	void _prepare_attached_regions(Genode::List<Stored_attached_region_info> &state_infos, Region_map_component  &child_obj);
	void _prepare_threads         (Genode::List<Stored_thread_info>          &state_infos, Cpu_session_component &child_obj);
	void _prepare_contexts        (Genode::List<Stored_signal_context_info>  &state_infos, Pd_session_component  &child_obj);
	void _prepare_sources         (Genode::List<Stored_signal_source_info>   &state_infos, Pd_session_component  &child_obj);
	void _prepare_region_map      (Stored_region_map_info                    &state_info,  Region_map_component  &child_obj);
	void _prepare_dataspaces      (Genode::List<Stored_dataspace_info>       &state_infos,
	                               Genode::List<Badge_dataspace_info>        &ds_infos,    Ram_session_component &child_obj);
	void _prepare_dataspaces      (Genode::List<Stored_dataspace_info>       &state_infos,
	                               Genode::List<Badge_dataspace_info>        &ds_infos,    Region_map_component  &child_obj);

	void _detach_designated_dataspaces(Ram_session_component &child_obj);

	void _checkpoint_dataspaces();
	void _update_normal_dataspace(Stored_dataspace_info &state_info, Ram_dataspace_info &child_info);
	void _update_managed_dataspace(Stored_dataspace_info &state_info, Managed_region_map_info &child_info);
	void _copy_dataspace_content(Genode::Dataspace_capability source_ds_cap, Genode::Dataspace_capability dest_ds_cap,
			Genode::size_t size, Genode::off_t dest_offset = 0);


public:
	Checkpointer(Genode::Allocator &alloc, Target_child &child, Target_state &state);
	Checkpointer(const Checkpointer &other) = delete;
	~Checkpointer();

	Checkpointer& operator=(const Checkpointer &other) = delete;


	/**
	 * Checkpoint all (known) RPC objects and capabilities from _child to _state
	 */
	void checkpoint();
};

#endif /* _RTCR_CHECKPOINTER_H_ */
