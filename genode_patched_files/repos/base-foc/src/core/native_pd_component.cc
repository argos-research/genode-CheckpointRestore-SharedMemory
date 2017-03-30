/*
 * \brief  Kernel-specific part of the PD-session interface
 * \author Norman Feske
 * \date   2016-01-19
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <pd_session_component.h>
#include <native_pd_component.h>

namespace Fiasco {
#include <l4/sys/task.h>
#include <l4/sys/ipc_gate.h>
}

using namespace Genode;


Native_capability Native_pd_component::task_cap()
{
	return Native_capability(_pd_session._pd.native_task());
}


// START Modification for Checkpoint/Restore (rtcr)
addr_t Native_pd_component::cap_map_info()
{
	return _cap_map_info;
}


void Native_pd_component::cap_map_info(addr_t addr)
{
	_cap_map_info = addr;
}

void Native_pd_component::install(Native_capability cap, addr_t kcap)
{
	using namespace Fiasco;

	//log("Native_pd::\033[33m", __func__, "\033[0m(", cap, ", kcap=", Hex(kcap), ")");

	// Testing whether remote task has a valid cap at the target cap selector
	{
		l4_msgtag_t tag = l4_task_cap_valid(task_cap().data()->kcap(), (l4_cap_idx_t) kcap);
		if(l4_msgtag_label(tag))
			warning("Overriding valid capability at kcap=", Hex(kcap));
	}

	// Mapping cap from core's cap space to the task associated with this native PD session
	{
		l4_msgtag_t tag = l4_task_map(task_cap().data()->kcap(), L4_BASE_TASK_CAP,
					l4_obj_fpage((l4_cap_idx_t)cap.data()->kcap(), 0, L4_FPAGE_RWX),
					((l4_cap_idx_t)kcap | L4_ITEM_MAP));

		if (l4_msgtag_has_error(tag))
			error("mapping cap failed");
	}
}
// END Modification for Checkpoint/Restore (rtcr)


Native_pd_component::Native_pd_component(Pd_session_component &pd_session,
                                         char const *args)
:
	_pd_session(pd_session), _cap_map_info(0)
{
	_pd_session._thread_ep.manage(this);
}


Native_pd_component::~Native_pd_component()
{
	_pd_session._thread_ep.dissolve(this);
}


