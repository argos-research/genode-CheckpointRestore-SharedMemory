/*
 * \brief  Target's state
 * \author Denis Huber
 * \date   2016-10-25
 */

#ifndef _RTCR_TARGET_STATE_H_
#define _RTCR_TARGET_STATE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <base/env.h>

/* Rtcr includes */
#include "child_state/stored_rm_session_info.h"
#include "child_state/stored_log_session_info.h"
#include "child_state/stored_timer_session_info.h"
#include "child_state/stored_region_map_info.h"
#include "child_state/stored_thread_info.h"
#include "child_state/stored_attached_region_info.h"
#include "child_state/stored_dataspace_info.h"
#include "child_state/stored_signal_context_info.h"
#include "child_state/stored_signal_source_info.h"

namespace Rtcr {
	class  Target_state;

	// Forward declaration
	class Checkpointer;
}

class Rtcr::Target_state
{
	friend class Checkpointer;
	friend class Restorer;

private:
	Genode::Env       &_env;
	Genode::Allocator &_alloc;

	Genode::List<Stored_rm_session_info>     _stored_rm_sessions;
	Genode::List<Stored_log_session_info>    _stored_log_sessions;
	Genode::List<Stored_timer_session_info>  _stored_timer_sessions;
	Genode::List<Stored_thread_info>         _stored_threads;
	Genode::List<Stored_region_map_info>     _stored_address_space;
	Genode::List<Stored_region_map_info>     _stored_stack_area;
	Genode::List<Stored_region_map_info>     _stored_linker_area;
	Genode::List<Stored_dataspace_info>      _stored_dataspaces;
	Genode::List<Stored_signal_context_info> _stored_signal_contexts;
	Genode::List<Stored_signal_source_info>  _stored_signal_sources;

	template <typename T>
	void _delete_list(Genode::List<T> &infos);
	void _delete_list(Genode::List<Stored_rm_session_info> &infos);
	void _delete_list(Genode::List<Stored_region_map_info> &infos);
	void _delete_list(Genode::List<Stored_dataspace_info> &infos);

	template <typename T>
	void _copy_list(Genode::List<T> &from_infos, Genode::List<T> &to_infos);
	void _copy_list(Genode::List<Stored_rm_session_info> &from_infos, Genode::List<Stored_rm_session_info> &to_infos);
	void _copy_list(Genode::List<Stored_region_map_info> &from_infos, Genode::List<Stored_region_map_info> &to_infos);
	void _copy_list(Genode::List<Stored_dataspace_info> &from_infos, Genode::List<Stored_dataspace_info> &to_infos);

	void _copy_dataspace(Genode::Dataspace_capability source_ds_cap, Genode::Dataspace_capability dest_ds_cap,
			Genode::size_t size, Genode::off_t dest_offset = 0);

public:
	Target_state(Genode::Env &env, Genode::Allocator &alloc);
	Target_state(Target_state &other);
	~Target_state();

	Target_state& operator=(const Target_state &other) = delete;
};

#endif /* _RTCR_TARGET_STATE_H_ */
