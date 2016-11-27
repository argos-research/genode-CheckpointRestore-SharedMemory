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
	Genode::List<Ckpt_resto_badge_info> _ckpt_to_resto_badges;
	Genode::List<Orig_copy_resto_info> _memory_to_restore;


	void _identify_recreate_pd_sessions(
			Genode::List<Pd_session_component> &pd_sessions, Genode::List<Stored_pd_session_info> &stored_pd_sessions);
	void _identify_recreate_signal_sources(
			Pd_session_component &pd_session, Genode::List<Stored_signal_source_info> &stored_signal_sources);
	void _identify_recreate_pd_sessions(
			Genode::List<Signal_context_info> &signal_contexts, Genode::List<Stored_signal_context_info> &stored_signal_contexts);



	void _restore_state_pd_sessions(Genode::List<Pd_session_component> &pd_sessions, Genode::List<Stored_pd_session_info> &stored_pd_sessions);


	template<typename RESTO, typename CKPT>
	CKPT &_find_stored_object(RESTO &session_obj, Genode::List<CKPT> &stored_sessions_infos)
	{
		Ckpt_resto_badge_info *cr_info = _ckpt_to_resto_badges.first();
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
		Ckpt_resto_badge_info *cr_info = _ckpt_to_resto_badges.first();
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
