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


	void _identify_recreate_pd_sessions(Genode::List<Pd_session_component> &pd_sessions, Genode::List<Stored_pd_session_info> &stored_pd_sessions);
	bool _corresponding_pd_sessions(Pd_session_component &pd_session, Stored_pd_session_info &stored_pd_session);



	void _restore_state_pd_sessions(Genode::List<Pd_session_component> &pd_sessions);


public:
	Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state);
	~Restorer();

	void restore();
};

#endif /* _RTCR_RESTORER_H_ */
