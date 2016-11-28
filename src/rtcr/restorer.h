/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_RESTORER_H_
#define _RTCR_RESTORER_H_

/* Rtcr includes */
#include "target_state.h"
#include "target_child.h"
#include "util/ckpt_resto_badge_info.h"
#include "util/orig_copy_resto_info.h"

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
	Target_child &_child;
	Target_state &_state;
	Genode::List<Ckpt_resto_badge_info> _ckpt_to_resto_infos;
	Genode::List<Orig_copy_resto_info> _memory_to_restore;

	template<typename T>
	void _destroy_list(Genode::List<T> &list);

	/****************************************
	 *** Identify or recreate RPC objects ***
	 ****************************************/
	void _identify_recreate_pd_sessions(
			Pd_root &pd_root, Genode::List<Stored_pd_session_info> &stored_pd_sessions);
	void _identify_recreate_signal_sources(
			Pd_session_component &pd_session, Genode::List<Stored_signal_source_info> &stored_signal_sources);
	void _identify_recreate_signal_contexts(
			Pd_session_component &pd_session, Genode::List<Stored_signal_context_info> &stored_signal_contexts);

	void _identify_recreate_ram_sessions(
			Ram_root &ram_root, Genode::List<Stored_ram_session_info> &stored_ram_sessions);
	void _identify_recreate_ram_dataspaces(
			Ram_session_component &ram_session, Genode::List<Stored_ram_dataspace_info> &stored_ram_dataspaces);
	Genode::List<Ckpt_resto_badge_info> _identify_ram_dataspaces(
			Genode::List<Ram_dataspace_info> &ram_dataspaces, Genode::List<Stored_ram_dataspace_info> &stored_ram_dataspaces);

	void _identify_recreate_cpu_sessions(
			Cpu_root &cpu_root, Genode::List<Stored_cpu_session_info> &stored_cpu_sessions,
			Genode::List<Pd_session_component> &pd_sessions);
	void _identify_recreate_cpu_threads(
			Cpu_session_component &cpu_session, Genode::List<Stored_cpu_thread_info> &stored_cpu_threads,
			Genode::List<Pd_session_component> &pd_sessions);

	void _identify_recreate_rm_sessions(
			Rm_root &rm_root, Genode::List<Stored_rm_session_info> &stored_rm_sessions);
	void _identify_recreate_region_maps(
			Rm_session_component &rm_session, Genode::List<Stored_region_map_info> &stored_region_maps);

	void _identify_recreate_log_sessions(
			Log_root &log_root, Genode::List<Stored_log_session_info> &stored_log_sessions);

	void _identify_recreate_timer_sessions(
			Timer_root &timer_root, Genode::List<Stored_timer_session_info> &stored_timer_sessions);


	/************************************
	 *** Restore state of RPC objects ***
	 ************************************/
	void _restore_state_pd_sessions(
			Genode::List<Pd_session_component> &pd_sessions, Genode::List<Stored_pd_session_info> &stored_pd_sessions);

	void _restore_state_ram_sessions(
			Genode::List<Ram_session_component> &ram_sessions, Genode::List<Stored_ram_session_info> &stored_ram_sessions);
	void _restore_state_ram_dataspaces(
			Genode::List<Ram_dataspace_info> &ram_dataspaces, Genode::List<Stored_ram_dataspace_info> &stored_ram_dataspaces);

	void _restore_state_cpu_sessions(
			Genode::List<Cpu_session_component> &cpu_sessions, Genode::List<Stored_cpu_session_info> &stored_cpu_sessions);

	void _restore_state_rm_sessions(
			Genode::List<Rm_session_component> &rm_sessions, Genode::List<Stored_rm_session_info> &stored_rm_sessions);

	void _restore_state_log_sessions(
			Genode::List<Log_session_component> &log_sessions, Genode::List<Stored_log_session_info> &stored_log_sessions);

	void _restore_state_timer_sessions(
			Genode::List<Timer_session_component> &timer_sessions, Genode::List<Stored_timer_session_info> &stored_timer_sessions);


	template<typename RESTO, typename CKPT>
	CKPT &_find_stored_object(RESTO &session_obj, Genode::List<CKPT> &stored_sessions_infos)
	{
		Ckpt_resto_badge_info *cr_info = _ckpt_to_resto_infos.first();
		if(cr_info) cr_info = cr_info->find_by_resto_badge(session_obj.cap().local_name());
		if(!cr_info)
		{
			Genode::error("Could not find translation for ckpt badge from resto badge ", session_obj.cap().local_name());
			throw Genode::Exception();
		}
		CKPT *stored_session_info = stored_sessions_infos.first();
		if(stored_session_info) stored_session_info = stored_session_info->find_by_badge(cr_info->ckpt_badge);
		if(!stored_session_info)
		{
			Genode::error("Could not find corresponding stored object for ckpt badge ", cr_info->ckpt_badge);
			throw Genode::Exception();
		}

		return *stored_session_info;
	}

	template<typename RESTO, typename CKPT>
	CKPT &_find_stored_object_info(RESTO &info_obj, Genode::List<CKPT> &stored_object_infos)
	{
		Ckpt_resto_badge_info *cr_info = _ckpt_to_resto_infos.first();
		if(cr_info) cr_info = cr_info->find_by_resto_badge(info_obj.cap.local_name());
		if(!cr_info)
		{
			Genode::error("Could not find translation for ckpt badge from resto badge ", info_obj.cap.local_name());
			throw Genode::Exception();
		}
		CKPT *stored_session_info = stored_object_infos.first();
		if(stored_session_info) stored_session_info = stored_session_info->find_by_badge(cr_info->ckpt_badge);
		if(!stored_session_info)
		{
			Genode::error("Could not find corresponding stored object for ckpt badge ", cr_info->ckpt_badge);
			throw Genode::Exception();
		}

		return *stored_session_info;
	}


public:
	Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state);
	~Restorer();

	void restore();
};

#endif /* _RTCR_RESTORER_H_ */
