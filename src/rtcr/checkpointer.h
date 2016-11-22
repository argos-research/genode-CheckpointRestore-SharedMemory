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
#include "intercept/region_map_component.h"
#include "child_state/stored_attached_region_info.h"
#include "child_state/stored_dataspace_info.h"
#include "child_state/stored_log_session_info.h"
#include "intercept/cpu_session.h"
#include "intercept/pd_session.h"
#include "intercept/ram_session.h"
#include "util/ref_badge.h"

namespace Rtcr {
	class Checkpointer;
	struct Badge_kcap_info;

	constexpr bool checkpointer_verbose_debug = false;
}

/**
 * Structure to store badge-kcap tuple from a capability map of a Target_child
 */
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

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "kcap=", Hex(kcap), ", badge=", badge);
	}
};


class Rtcr::Checkpointer
{
private:
	/**
	 * Stores dataspace capabilities of designated_dataspace_info for checkpointing dataspaces
	 */
	struct Managed_dataspace_info : Genode::List<Managed_dataspace_info>::Element
	{
		Genode::Dataspace_capability ds_cap;
		Genode::addr_t rel_addr;
		Genode::size_t size;
		bool           checkpointed;
		Managed_dataspace_info(Genode::Dataspace_capability ds_cap, Genode::addr_t rel_addr, Genode::size_t size)
		: ds_cap(ds_cap), rel_addr(rel_addr), size(size), checkpointed(false) { }
	};

	/**
	 * Associate a valid badge to a valid dataspace capability
	 */
	struct Badge_dataspace_info : Genode::List<Badge_dataspace_info>::Element
	{
		Genode::uint16_t             badge;
		Genode::Dataspace_capability ds_cap;
		bool                         checkpointed;
		Genode::List<Managed_dataspace_info> infos;
		Badge_dataspace_info(Genode::uint16_t badge, Genode::Dataspace_capability ds_cap)
		: badge(badge), ds_cap(ds_cap), checkpointed(false), infos() { }

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
	Genode::List<Badge_kcap_info> _capability_map_infos;

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
	void _prepare_cap_map_infos(Genode::List<Badge_kcap_info> &state_infos);
	/**
	 * \brief If ar_info is managed, mark the detached dataspaces by badge, and attach them
	 *
	 * If ar_info contains a managed dataspace (allocated through the custom RAM session), then mark the detached
	 * designated dataspaces and attach them. Now the managed dataspace can be used directly without triggering page faults.
	 * The marked dataspaces have to be detached to not unnecessarily checkpoint them later
	 */
	Genode::List<Ref_badge> _mark_attach_designated_dataspaces(Attached_region_info &ar_info);
	/**
	 * \brief If ar_info is managed AND badge_infos is not empty, detach the previously attached dataspaces and
	 * delete the badge_infos' elements
	 *
	 * If the ar_info contains a managed dataspace (allocated through the custom RAM session) AND badge_infos is
	 * not empty, then detach the marked designated dataspaces and delete badge_infos' elements
	 */
	void _detach_unmark_designated_dataspaces(Genode::List<Ref_badge> &badge_infos, Attached_region_info &ar_info);
	/**
	 * \brief Return the kcap for a given badge from _capability_map_infos
	 *
	 * Return the kcap for a given badge. If there is no, return 0.
	 */
	Genode::addr_t find_kcap_by_badge(Genode::uint16_t badge);
	/**
	 * \brief Prepares the RM session list named state_infos
	 *
	 * First all list elements of state_info which have a corresponding list element in the child_obj are updated
	 * (if they do not exist, they are created).
	 * Second all old list elements of state_info which do not have a corresponding list element in the child_obj are deleted.
	 */
	void _prepare_rm_sessions(Genode::List<Stored_rm_session_info> &state_infos, Rm_root &child_obj);
	/**
	 * \brief Prepare the LOG session list named state_infos
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_log_sessions(Genode::List<Stored_log_session_info> &state_infos, Log_root &child_obj);
	/**
	 * \brief Prepare the Timer session list named state_infos
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_timer_sessions(Genode::List<Stored_timer_session_info> &state_infos, Timer_root &child_obj);
	/**
	 * \brief Prepare the region map list named state_infos
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_region_maps(Genode::List<Stored_region_map_info> &state_infos, Rm_session_component &child_obj);
	/**
	 * \brief Prepare the attached region list named state_infos
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_attached_regions(Genode::List<Stored_attached_region_info> &state_infos,
			Region_map_component &child_obj);
	/**
	 * \brief Prepare a specific CPU session named state_info
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_cpu_session(Stored_cpu_session_info &state_info, Cpu_session_component &child_obj);
	/**
	 * \brief Prepare the thread list named state_infos
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_threads(Genode::List<Stored_thread_info> &state_infos, Cpu_session_component &child_obj);
	/**
	 * \brief Prepare a specific PD session named state_info
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_pd_session(Stored_pd_session_info &state_info, Pd_session_component &child_obj);
	/**
	 * \brief Prepare the signal context list named state_infos
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_contexts(Genode::List<Stored_signal_context_info> &state_infos, Pd_session_component &child_obj);
	/**
	 * \brief Prepare the signal source list named state_infos
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_sources(Genode::List<Stored_signal_source_info> &state_infos, Pd_session_component &child_obj);
	/**
	 * \brief Prepare a specific region map named state_info
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_region_map(Stored_region_map_info &state_info,  Region_map_component &child_obj);
	/**
	 * \brief Prepare a specific RAM session named state_info
	 *
	 * For a detailed description there are comments in the method or refer to the description of _prepare_rm_sessions
	 */
	void _prepare_ram_session(Stored_ram_session_info &state_info, Ram_session_component &child_obj);
	void _prepare_allocated_dataspaces(Genode::List<Ref_badge> &state_infos, Ram_session_component &child_obj);

	/**
	 * \brief Update the dataspace list named state_infos by allocated dataspaces
	 *
	 * All list elements of state_infos which have a corresponding list element in child_obj are updated. If no corresponding
	 * state_info exists, create it. Also mark every state_info, which was updated, in visited_infos.
	 */
	void _update_dataspace_infos(Genode::List<Stored_dataspace_info> &state_infos, Ram_session_component &child_obj,
			Genode::List<Badge_dataspace_info> &visited_infos);
	/**
	 * \brief Update the dataspace list named state_infos by attached dataspaces of standard region maps
	 *
	 * Update (or create) a not excluded (found in exclude) and not yet seen (found in visited_infos) state_info list element,
	 * which corresponds to a list element of child_obj. Also mark every state_info, which was update, in visited_infos.
	 */
	void _update_dataspace_infos(Genode::List<Stored_dataspace_info> &state_infos, Region_map_component &child_obj,
			Genode::List<Badge_dataspace_info> &visited_infos, Genode::List<Ref_badge> &exclude);
	/**
	 * \brief Update the dataspace list named state_infos by attached dataspaces of extra region maps
	 *
	 * For each region map in an RM session and each RM session in an RM root, call
	 * _update_dataspace_infos(..., Region_map_component, ...)
	 */
	void _update_dataspace_infos(Genode::List<Stored_dataspace_info> &state_infos, Rm_root &child_obj,
			Genode::List<Badge_dataspace_info> &visited_infos, Genode::List<Ref_badge> &exclude);
	/**
	 * \brief Delete old list elements from dataspace list named state_infos
	 *
	 * Delete all unmarked (not found in visited_infos) list elements
	 */
	void _delete_old_dataspace_infos(Genode::List<Stored_dataspace_info> &state_infos,
			Genode::List<Badge_dataspace_info> &visited_infos);
	/**
	 * Create a list of dataspace capabilities which belong to a managed dataspace (i.e. region map dataspace). This list is used
	 * to identify region maps which are attached to another region map (e.g. the address space) to copy their dataspaces.
	 */
	Genode::List<Ref_badge> _create_exclude_infos(Region_map_component &address_space, Region_map_component &stack_area,
			Region_map_component &linker_area, Rm_root *rm_root);
	/**
	 * Destroy a list which represents dataspace capabilities belonging to a managed dataspace.
	 * See _delete_old_dataspace_infos.
	 */
	void _destroy_exclude_infos(Genode::List<Ref_badge> &infos);
	/**
	 * Detach all designated dataspaces to get a notification for the incremental checkpoint
	 */
	void _detach_designated_dataspaces(Ram_session_component &child_obj);
	/**
	 * \brief Checkpoints the content of dataspaces
	 *
	 * Requirements: All state_infos's list elements are synchronised/up-to-date with the child's.
	 * Copies content from the dataspaces from the child to the dataspaces of the state.
	 * The child is not allowed to free its allocated dataspaces before the dataspace was checkpointed.
	 */
	void _checkpoint_dataspaces(Genode::List<Stored_dataspace_info> &state_infos,
			Genode::List<Badge_dataspace_info> &ds_infos);
	/**
	 * Checkpoint a normal dataspace completely. Sets the dataspace as checkpointed.
	 */
	void _update_normal_dataspace(Stored_dataspace_info &state_info, Badge_dataspace_info &visited_info);
	/**
	 * Checkpoint a dirtied region of a managed dataspace. Sets the dataspace as checkpointed.
	 */
	void _update_managed_dataspace(Stored_dataspace_info &state_info, Managed_dataspace_info &managed_ds_infos);
	/**
	 * Copy the content of a source dataspace to a destination dataspace starting with an offset
	 * in the destination dataspace.
	 */
	void _copy_dataspace_content(Genode::Dataspace_capability src_ds_cap, Genode::Dataspace_capability dst_ds_cap,
			Genode::size_t size, Genode::off_t dst_offset = 0);


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
