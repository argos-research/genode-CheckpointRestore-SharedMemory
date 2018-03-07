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
#include <foc_native_pd/client.h>

/* Rtcr includes */
#include "target_state.h"
#include "target_child.h"
#include "util/kcap_badge_info.h"
#include "util/dataspace_translation_info.h"
#include "util/ref_badge_info.h"
#include "util/simplified_managed_dataspace_info.h"

namespace Rtcr {
	class Checkpointer;

	constexpr bool checkpointer_verbose_debug = true;
}


class Rtcr::Checkpointer
{
private:

	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = checkpointer_verbose_debug;
	/**
	 * Allocator for checkpointer's personal datastructures. The datastructures which belong to Target_state
	 * are created with the allocator of Target_state
	 */
	Genode::Allocator &_alloc;
	/**
	 * Target_child which shall be checkpointed
	 */
	Target_child      &_child;
	/**
	 * Target_state where the checkpointed state is stored
	 */
	Target_state      &_state;
	/**
	 * Capability map of Target_child in a condensed form
	 */
	Genode::List<Kcap_badge_info>      _kcap_mappings;
	/**
	 * Mapping to find a copy dataspace for a given original dataspace badge
	 */
	Genode::List<Dataspace_translation_info> _dataspace_translations;
	/**
	 * List of dataspace badges which are (known) managed dataspaces
	 * These dataspaces are not needed to be copied
	 */
	Genode::List<Ref_badge_info>            _region_maps;
	Genode::List<Simplified_managed_dataspace_info> _managed_dataspaces;


	template<typename T>
	void _destroy_list(Genode::List<T> &list);
	void _destroy_list(Genode::List<Simplified_managed_dataspace_info> &list);

	/**
	 * \brief Prepares the capability map state_infos
	 *
	 * First the method fetches the capability map information from child's cap map structure in an
	 * intercepted dataspace.
	 *
	 * Second it prepares the capability map state_infos.
	 * For each badge-kcap tuple found in child's cap map the method checks whether a corresponding
	 * list element in state_infos exists. If there is no list element, then it is created and marked.
	 * Else it is just marked. After this, the old badge-kcap tuples, which where not marked, are deleted
	 * from state_infos. Now an updated capability map is ready to used for the next steps to store the
	 * kcap for each RPC object.
	 */
	Genode::List<Kcap_badge_info> _create_kcap_mappings();
	Genode::List<Ref_badge_info> _mark_and_attach_designated_dataspaces(Attached_region_info &ar_info);
	void _detach_and_unmark_designated_dataspaces(Genode::List<Ref_badge_info> &badge_infos, Attached_region_info &ar_info);
	/**
	 * \brief Return the kcap for a given badge from _capability_map_infos
	 *
	 * Return the kcap for a given badge. If there is no, return 0.
	 */
	Genode::addr_t _find_kcap_by_badge(Genode::uint16_t badge);

	/**
	 * Searches for a dataspace which stores the content of a child's dataspace in ALL possible session RPC objects
	 */
	Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge);
	Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge, Genode::List<Stored_ram_session_info> &state_infos);
	Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge, Genode::List<Stored_pd_session_info> &state_infos);
	Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge, Genode::List<Stored_rm_session_info> &state_infos);
	Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge, Genode::List<Stored_attached_region_info> &state_infos);

	void _prepare_rm_sessions(Genode::List<Stored_rm_session_info> &stored_infos, Genode::List<Rm_session_component> &child_infos);
	void _destroy_stored_rm_session(Stored_rm_session_info &stored_info);

	void _prepare_region_maps(Genode::List<Stored_region_map_info> &stored_infos, Genode::List<Region_map_component> &child_infos);
	void _destroy_stored_region_map(Stored_region_map_info &stored_info);

	void _prepare_attached_regions(Genode::List<Stored_attached_region_info> &stored_infos, Genode::List<Attached_region_info> &child_infos);
	Stored_attached_region_info &_create_stored_attached_region(Attached_region_info &child_info);
	void _destroy_stored_attached_region(Stored_attached_region_info &stored_info);

	void _prepare_ram_sessions(Genode::List<Stored_ram_session_info> &stored_infos, Genode::List<Ram_session_component> &child_infos);
	void _destroy_stored_ram_session(Stored_ram_session_info &stored_info);

	void _prepare_ram_dataspaces(Genode::List<Stored_ram_dataspace_info> &stored_infos, Genode::List<Ram_dataspace_info> &child_infos);
	Stored_ram_dataspace_info &_create_stored_ram_dataspace(Ram_dataspace_info &child_info);
	void _destroy_stored_ram_dataspace(Stored_ram_dataspace_info &stored_info);

	void _prepare_cpu_sessions(Genode::List<Stored_cpu_session_info> &stored_infos, Genode::List<Cpu_session_component> &child_infos);
	void _destroy_stored_cpu_session(Stored_cpu_session_info &stored_info);

	void _prepare_cpu_threads(Genode::List<Stored_cpu_thread_info> &stored_infos, Genode::List<Cpu_thread_component> &child_infos);
	void _destroy_stored_cpu_thread(Stored_cpu_thread_info &stored_info);

	void _prepare_pd_sessions(Genode::List<Stored_pd_session_info> &stored_infos, Genode::List<Pd_session_component> &child_infos);
	void _destroy_stored_pd_session(Stored_pd_session_info &stored_info);

	void _prepare_native_caps(Genode::List<Stored_native_capability_info> &stored_infos, Genode::List<Native_capability_info> &child_infos);
	void _destroy_stored_native_cap(Stored_native_capability_info &stored_info);

	void _prepare_signal_sources(Genode::List<Stored_signal_source_info> &stored_infos, Genode::List<Signal_source_info> &child_infos);
	void _destroy_stored_signal_source(Stored_signal_source_info &stored_info);

	void _prepare_signal_contexts(Genode::List<Stored_signal_context_info> &stored_infos, Genode::List<Signal_context_info> &child_infos);
	void _destroy_stored_signal_context(Stored_signal_context_info &stored_info);

	void _prepare_log_sessions(Genode::List<Stored_log_session_info> &stored_infos, Genode::List<Log_session_component> &child_infos);
	void _destroy_stored_log_session(Stored_log_session_info &stored_info);

	void _prepare_timer_sessions(Genode::List<Stored_timer_session_info> &stored_infos, Genode::List<Timer_session_component> &child_infos);
	void _destroy_stored_timer_session(Stored_timer_session_info &stored_info);

	Genode::List<Ref_badge_info> _create_region_map_dataspaces_list(
			Genode::List<Pd_session_component> &pd_sessions, Genode::List<Rm_session_component> *rm_sessions);

	void _create_managed_dataspace_list(Genode::List<Ram_session_component> &ram_sessions);

	void _detach_designated_dataspaces(Genode::List<Ram_session_component> &ram_sessions);

	void _checkpoint_redundant_dataspaces(Genode::List<Ram_session_component> &ram_sessions);

	void _lock_redundant_dataspaces(bool lock);

	void _checkpoint_dataspaces();
	void _checkpoint_dataspace_content(Genode::Dataspace_capability dst_ds_cap, Genode::Dataspace_capability src_ds_cap,
			Genode::addr_t dst_offset, Genode::size_t size);

public:
	Checkpointer(Genode::Allocator &alloc, Target_child &child, Target_state &state);
	~Checkpointer();

	void activate_redundant_memory();

	/**
	 * Checkpoint all (known) RPC objects and capabilities from _child to _state
	 */
	void checkpoint();
};

#endif /* _RTCR_CHECKPOINTER_H_ */
