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


/*Native_capability Native_pd_component::request(addr_t kcap)
{
	using namespace Fiasco;

	log("Hello from ", __func__);

	Capability<Native_pd> to_pd;
	addr_t to_sel;

	// Read capability from this pd to remote pd directly
	l4_msgtag_t tag = l4_task_map(to_pd.data()->kcap(), _pd_session.native_pd().data()->kcap(),
			l4_obj_fpage((l4_cap_idx_t)from_sel, 0, L4_FPAGE_RWX),
			((l4_cap_idx_t)to_sel | L4_ITEM_MAP));

	if (l4_msgtag_has_error(tag))
		error("mapping cap failed");


	// 1. Create temporary Cap_index
	Cap_index *idx = cap_map()->insert(platform_specific()->cap_id_alloc()->alloc());
	printf("Cap_index: %d %x\n", idx->id(), idx->kcap());

	// 2. Map target's Cap_index to temporary Cap_index
	l4_msgtag_t tag = l4_task_map(L4_BASE_TASK_CAP, _pd_session.native_pd().data()->kcap(),
			l4_obj_fpage((l4_cap_idx_t)kcap, 0, L4_FPAGE_RWX),
			idx->kcap() | L4_ITEM_MAP);

	if (l4_msgtag_has_error(tag))
		error("mapping cap failed");

	printf("Cap_index: %d %x\n", idx->id(), idx->kcap());

	// 3. Find temporary Cap_index in Capability_map
	//cap_map()->find();


	// 4. Delete temporary Cap_index
	//cap_idx_alloc()->free(idx, 1);

	// 5. Return found capability
	Native_capability cap = Native_capability(*idx);
	printf("Data: %d %x, Cap.local_name: %d\n", cap.data()->id(), cap.data()->kcap(), cap.local_name());
	return cap;
}*/

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
	log("Implement me: ", __func__);
}


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


