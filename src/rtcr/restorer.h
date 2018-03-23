/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_RESTORER_H_
#define _RTCR_RESTORER_H_

/* Genode includes */
#include <foc_native_pd/client.h>

/* Rtcr includes */
#include "target_state.h"
#include "target_child.h"
#include "util/kcap_cap_info.h"
#include "util/badge_translation_info.h"
#include "util/dataspace_translation_info.h"
#include "util/ref_badge_info.h"
#include "util/simplified_managed_dataspace_info.h"

namespace Rtcr {
	class Restorer;

	constexpr bool restorer_verbose_debug = true;
}

class Rtcr::Restorer
{
private:

	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = restorer_verbose_debug;
	/**
	 * Allocator for restorer's personal datastructures. The datastructures which belong to Target_state
	 * are created with the allocator of Target_state
	 */
	Genode::Allocator &_alloc;
	Target_child      &_child;
	Target_state      &_state;
	/**
	 * \brief Contains kcap addresses and badges/capabilities which shall be mapped to the kcap addresses
	 *
	 * Contains kcap addresses and badges/capabilities which shall be mapped to these addresses
	 * Usage
	 * * Restoration of the capability map: Store the badge to the corresponding kcap address
	 * * Restoration of the cap space: Map the capability to the corresponding kcap address
	 */
	Genode::List<Kcap_cap_info> _kcap_mappings;
	/**
	 * \brief Contains mappings of stored RPC objects to newly recreated RPC objects via badge association
	 *
	 * Each stored RPC object has an associated restored RPC object. This list contains a mapping of a badge
	 * of the stored RPC object and the corresponding capability of the newly created RPC object
	 */
	Genode::List<Badge_translation_info> _rpcobject_translations;
	/**
	 * \brief Contains mappings of stored dataspaces to newly recreated dataspaces via capability association
	 *
	 * Each stored dataspace has a corresponding restored dataspace. The content of the stored dataspace has to be
	 * copied to the corresponding dataspace. Both dataspaces are associated via capabilities.
	 */
	Genode::List<Dataspace_translation_info> _dataspace_translations;
	/**
	 * \brief Contains badges of region maps
	 *
	 * This list contains badges of dataspace capabilities of region maps. They are used to identify region maps
	 * which are attached to other region maps in order to not confuse them with real dataspaces.
	 */
	Genode::List<Ref_badge_info> _region_maps;
	/**
	 * \brief Contains the managed dataspaces of the incremental checkpointing and their designated dataspaces
	 *
	 * This list contains the managed dataspaces used by the incremental checkpointing mechanism. It is used for
	 * restoring the state of the dataspaces (i.e. copying memory content) to use directly the designated dataspaces
	 * instead of triggering page faults by using the managed dataspace directly
	 */
	Genode::List<Simplified_managed_dataspace_info> _managed_dataspaces;

	template<typename T>
	void _destroy_list(Genode::List<T> &list);
	void _destroy_list(Genode::List<Simplified_managed_dataspace_info> &list);

	Genode::List<Ref_badge_info> _create_region_map_dataspaces_list(
			Genode::List<Stored_pd_session_info> &stored_pd_sessions, Genode::List<Stored_rm_session_info> &stored_rm_sessions);

	/****************************************
	 *** Identify or recreate RPC objects ***
	 ****************************************/
	void _identify_recreate_pd_sessions(
			Pd_root &pd_root, Genode::List<Stored_pd_session_info> &stored_pd_sessions);
	void _recreate_signal_sources(
			Pd_session_component &pd_session, Genode::List<Stored_signal_source_info> &stored_signal_sources);
	void _recreate_signal_contexts(
			Pd_session_component &pd_session, Genode::List<Stored_signal_context_info> &stored_signal_contexts);

	void _identify_recreate_ram_sessions(
			Ram_root &ram_root, Genode::List<Stored_ram_session_info> &stored_ram_sessions);
	void _recreate_ram_dataspaces(
			Ram_session_component &ram_session, Genode::List<Stored_ram_dataspace_info> &stored_ram_dataspaces);

	void _identify_recreate_cpu_sessions(
			Cpu_root &cpu_root, Genode::List<Stored_cpu_session_info> &stored_cpu_sessions,
			Genode::List<Pd_session_component> &pd_sessions);
	void _identify_recreate_cpu_threads(
			Cpu_session_component &cpu_session, Genode::List<Stored_cpu_thread_info> &stored_cpu_threads,
			Genode::List<Pd_session_component> &pd_sessions);

	void _recreate_rm_sessions(
			Rm_root &rm_root, Genode::List<Stored_rm_session_info> &stored_rm_sessions);
	void _recreate_region_maps(
			Rm_session_component &rm_session, Genode::List<Stored_region_map_info> &stored_region_maps);

	void _recreate_log_sessions(
			Log_root &log_root, Genode::List<Stored_log_session_info> &stored_log_sessions);

	void _recreate_timer_sessions(
			Timer_root &timer_root, Genode::List<Stored_timer_session_info> &stored_timer_sessions);


	/************************************
	 *** Restore state of RPC objects ***
	 ************************************/
	void _restore_state_pd_sessions(
			Pd_root &pd_root, Genode::List<Stored_pd_session_info> &stored_pd_sessions);

	void _restore_state_ram_sessions(
			Ram_root &ram_root, Genode::List<Stored_ram_session_info> &stored_ram_sessions);
	void _restore_state_ram_dataspaces(
			Ram_session_component &ram_session, Genode::List<Stored_ram_dataspace_info> &stored_ram_dataspaces);

	void _restore_state_cpu_sessions(
			Cpu_root &cpu_root, Genode::List<Stored_cpu_session_info> &stored_cpu_sessions,
			Genode::List<Pd_session_component> &pd_sessions);
	void _restore_state_cpu_threads(
			Cpu_session_component &cpu_session, Genode::List<Stored_cpu_thread_info> &stored_cpu_threads,
			Genode::List<Pd_session_component> &pd_sessions);

	void _restore_state_rm_sessions(
			Rm_root &rm_root, Genode::List<Stored_rm_session_info> &stored_rm_sessions,
			Genode::List<Pd_session_component> &pd_sessions);
	void _restore_state_region_maps(
			Genode::List<Region_map_component> &region_maps, Genode::List<Stored_region_map_info> &stored_region_maps,
			Genode::List<Pd_session_component> &pd_sessions);
	void _restore_state_attached_regions(
			Region_map_component &region_map, Genode::List<Stored_attached_region_info> &stored_attached_regions);

	void _restore_state_log_sessions(
			Log_root &log_root, Genode::List<Stored_log_session_info> &stored_log_sessions);

	void _restore_state_timer_sessions(
			Timer_root &timer_root, Genode::List<Stored_timer_session_info> &stored_timer_sessions,
			Genode::List<Pd_session_component> &pd_sessions);

	template<typename RESTO>
	RESTO *_find_child_object(Genode::uint16_t badge, Genode::List<RESTO> &child_objects)
	{
		Badge_translation_info *trans_info = _rpcobject_translations.first();
		if(trans_info) trans_info = trans_info->find_by_ckpt_badge(badge);
		if(!trans_info)
		{
			Genode::error("Could not translate stored badge ", badge);
			throw Genode::Exception();
		}
		RESTO *child_object = child_objects.first();
		if(child_object) child_object = child_object->find_by_badge(trans_info->resto_cap.local_name());

		return child_object;
	}

	void _create_managed_dataspace_list(Genode::List<Ram_session_component> &ram_sessions);

	void _restore_cap_map();
	void _restore_cap_space();

	void _restore_dataspaces();
	void _restore_dataspaces_redundant_memory();
	void _restore_dataspace_content(Genode::Dataspace_capability dst_ds_cap,
			Genode::Dataspace_capability src_ds_cap, Genode::addr_t src_offset, Genode::size_t size);
	void _restore_redundant_dataspace_content(Genode::Dataspace_capability dst_ds_cap,
			Rtcr::Designated_redundant_ds_info& src_drdsi, Genode::addr_t src_offset, Genode::size_t size);

	void _start_threads(
			Cpu_root &cpu_root, Genode::List<Stored_cpu_session_info> &stored_cpu_sessions);


public:
	Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state);
	~Restorer();

	void restore();
};

#endif /* _RTCR_RESTORER_H_ */
