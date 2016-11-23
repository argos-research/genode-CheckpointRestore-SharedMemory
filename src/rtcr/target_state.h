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

/* Rtcr includes */
#include "child_state/stored_pd_session_info.h"
#include "child_state/stored_cpu_session_info.h"
#include "child_state/stored_ram_session_info.h"
#include "child_state/stored_rom_session_info.h"
#include "child_state/stored_rm_session_info.h"
#include "child_state/stored_log_session_info.h"
#include "child_state/stored_timer_session_info.h"


namespace Rtcr {
	class Target_state;

	// Forward declaration
	class Checkpointer;
	class Restorer;
}

class Rtcr::Target_state
{
	friend class Checkpointer;
	friend class Restorer;

private:
	Genode::Ram_session &_ram;
	Genode::Allocator   &_alloc;

	Genode::List<Stored_pd_session_info>     _stored_pd_sessions;
	Genode::List<Stored_cpu_session_info>    _stored_cpu_sessions;
	Genode::List<Stored_ram_session_info>    _stored_ram_sessions;
	Genode::List<Stored_rom_session_info>    _stored_rom_sessions;
	Genode::List<Stored_rm_session_info>     _stored_rm_sessions;
	Genode::List<Stored_log_session_info>    _stored_log_sessions;
	Genode::List<Stored_timer_session_info>  _stored_timer_sessions;

public:
	Target_state(Genode::Ram_session &ram, Genode::Allocator &alloc);
	~Target_state();

	void print(Genode::Output &output) const;
};

#endif /* _RTCR_TARGET_STATE_H_ */
