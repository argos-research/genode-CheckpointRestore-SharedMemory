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
}

using namespace Genode;


Native_capability Native_pd_component::task_cap()
{
	return Native_capability(_pd_session._pd.native_task());
}


Native_capability Native_pd_component::request(Native_capability to_pd, addr_t to_sel, addr_t from_sel)
{
	using namespace Fiasco;

	log("Hello from ", __func__);

	// Read capability from this pd to remote pd directly
	l4_task_map(to_pd.data()->kcap(), _pd_session.native_pd().data()->kcap(),
			l4_obj_fpage((l4_cap_idx_t)from_sel, 0, L4_FPAGE_RWX),
			((l4_cap_idx_t)to_sel | L4_ITEM_MAP));

	// 1. Read capability from this pd to core's pd
	l4_task_map(L4_BASE_TASK_CAP, _pd_session.native_pd().data()->kcap(),
			l4_obj_fpage((l4_cap_idx_t)from_sel, 0, L4_FPAGE_RWX),
			L4_ITEM_MAP);
	// 2. Return Native_capability

	return Native_capability();
}


void Native_pd_component::install(Native_capability cap, uint32_t selector)
{
	log("Implement me: ", __func__);
}


Native_pd_component::Native_pd_component(Pd_session_component &pd_session,
                                         char const *args)
:
	_pd_session(pd_session)
{
	_pd_session._thread_ep.manage(this);
}


Native_pd_component::~Native_pd_component()
{
	_pd_session._thread_ep.dissolve(this);
}


