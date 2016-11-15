/*
 * \brief  Checkpointer of Target_state
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "checkpointer.h"
#include "util/debug.h"
#include "base/internal/cap_map.h"
#include "base/internal/cap_alloc.h"

using namespace Rtcr;


void Checkpointer::_prepare_cap_map_infos(Genode::List<Badge_kcap_info> &state_infos)
{
	using Genode::log;
	using Genode::Hex;
	using Genode::addr_t;

	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
	Genode::log(__func__, ": Implementing...");

	Genode::addr_t cap_map_addr = Genode::Foc_native_pd_client(_child.pd().native_pd()).cap_map_addr();
	log("from ckpt: ", Genode::Hex(cap_map_addr));

	Attached_region_info *info = _child.pd().address_space_component().attached_regions().first();
	if(info) info = info->find_by_addr_and_exec(cap_map_addr, false);
	if(info)
	{
		log(*info);

		unsigned char *attached_ptr = _state._env.rm().attach(info->ds_cap);
		log("Attached ds at: ", attached_ptr);
		const addr_t local_start = (addr_t)attached_ptr;
		const addr_t child_start = (addr_t)info->rel_addr;

		addr_t local_cap_map_addr = cap_map_addr - child_start + local_start;
		addr_t local_first = *(addr_t*)local_cap_map_addr - child_start + local_start;

		log("Size of cap_idx_alloc_tpl: ", Hex(sizeof(Genode::Cap_index_allocator_tpl<Genode::Cap_index,4096>)));

		// Capability map consists of list and spin_lock, each occupying 4 bytes
		//log("Size of Capability_map: ", sizeof(Genode::Capability_map));
		log("cap_map_addr: ", Hex(cap_map_addr));
		dump_mem((void*)local_cap_map_addr, 8);
		// first Cap_index consists of next ptr (4 byte), ref_count (1 byte), and badge (2 byte)
		//log("Size of Cap_index: ", sizeof(Genode::Cap_index));
		log("first: ", Hex(*(addr_t*)local_cap_map_addr));
		dump_mem((void*)local_first, 8);

		dump_mem(Genode::cap_idx_alloc(), 0x1000);
		Genode::Cap_index *ci = Genode::cap_idx_alloc()->alloc_range(1);
		log(ci);
		//dump_mem(((char*)ci)-0x1000, 0x2000);
	}
	else
	{
		Genode::log("not found");
	}
}


void Checkpointer::_prepare_rm_sessions(Genode::List<Stored_rm_session_info> &state_infos, Rm_root &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Rm_session_info *child_info = nullptr;
	Stored_rm_session_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.rms_infos().first();
	while(child_info)
	{
		// Find corresponding state_info
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->session.cap().local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_rm_session_info(*child_info);
			state_infos.insert(state_info);
		}

		// Update state_info
		_prepare_region_maps(state_info->stored_region_map_infos, child_info->session);

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		Stored_rm_session_info *next_info = state_info->next();

		// Find corresponding child_info
		child_info = child_obj.rms_infos().first();
		while(child_info)
		{
			if(state_info->badge == child_info->session.cap().local_name()) break;
			child_info = child_info->next();
		}

		// No corresponding child_info => delete it
		if(!child_info)
		{
			state_infos.remove(state_info);
			Genode::destroy(_state._alloc, state_info);
		}

		state_info = next_info;
	}
}


void Checkpointer::_prepare_log_sessions(Genode::List<Stored_log_session_info> &state_infos, Log_root &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Log_session_info *child_info = nullptr;
	Stored_log_session_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.session_infos().first();
	while(child_info)
	{
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->session.cap().local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_log_session_info(*child_info);
			state_infos.insert(state_info);
		}

		// No need to update state_info

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		Stored_log_session_info *next_info = state_info->next();

		// Find corresponding child_info
		child_info = child_obj.session_infos().first();
		while(child_info)
		{
			if(state_info->badge == child_info->session.cap().local_name()) break;
			child_info = child_info->next();
		}

		// No corresponding child_info => delete it
		if(!child_info)
		{
			state_infos.remove(state_info);
			Genode::destroy(_state._alloc, state_info);
		}

		state_info = next_info;
	}
}


void Checkpointer::_prepare_timer_sessions(Genode::List<Stored_timer_session_info> &state_infos, Timer_root &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Timer_session_info *child_info = nullptr;
	Stored_timer_session_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.session_infos().first();
	while(child_info)
	{
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->session.cap().local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_timer_session_info(*child_info);
			state_infos.insert(state_info);
		}

		// Update state_info
		state_info->sigh_badge = child_info->session.parent_state().sigh.local_name();
		state_info->timeout = child_info->session.parent_state().timeout;
		state_info->periodic = child_info->session.parent_state().periodic;

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		Stored_timer_session_info *next_info = state_info->next();

		// Find corresponding child_info
		child_info = child_obj.session_infos().first();
		while(child_info)
		{
			if(state_info->badge == child_info->session.cap().local_name()) break;
			child_info = child_info->next();
		}

		// No corresponding child_info => delete it
		if(!child_info)
		{
			state_infos.remove(state_info);
			Genode::destroy(_state._alloc, state_info);
		}

		state_info = next_info;
	}
}


void Checkpointer::_prepare_region_maps(Genode::List<Stored_region_map_info> &state_infos, Rm_session_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Region_map_info *child_info = nullptr;
	Stored_region_map_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.region_map_infos().first();
	while(child_info)
	{
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->region_map.cap().local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_region_map_info(*child_info);
			state_infos.insert(state_info);
		}

		// Update state_info
		state_info->fault_handler_badge = child_info->region_map.parent_state().fault_handler.local_name();
		_prepare_attached_regions(state_info->stored_attached_region_infos, child_info->region_map);

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		Stored_region_map_info *next_info = state_info->next();

		// Find corresponding child_info
		child_info = child_obj.region_map_infos().first();
		while(child_info)
		{
			if(state_info->badge == child_info->region_map.cap().local_name()) break;
			child_info = child_info->next();
		}

		// No corresponding child_info => delete it
		if(!child_info)
		{
			state_infos.remove(state_info);
			Genode::destroy(_state._alloc, state_info);
		}

		state_info = next_info;
	}
}


void Checkpointer::_prepare_attached_regions(Genode::List<Stored_attached_region_info> &state_infos, Region_map_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Attached_region_info *child_info = nullptr;
	Stored_attached_region_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.attached_regions().first();
	while(child_info)
	{
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge_and_addr(child_info->ds_cap.local_name(), child_info->rel_addr);

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_attached_region_info(*child_info);
			state_infos.insert(state_info);
		}

		// No need to update state_info

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		Stored_attached_region_info *next_info = state_info->next();

		// Find corresponding child_info
		child_info = child_obj.attached_regions().first();
		while(child_info)
		{
			if(state_info->badge == child_info->ds_cap.local_name() && state_info->rel_addr == child_info->rel_addr) break;
			child_info = child_info->next();
		}

		// No corresponding child_info => delete it
		if(!child_info)
		{
			state_infos.remove(state_info);
			Genode::destroy(_state._alloc, state_info);
		}

		state_info = next_info;
	}
}


void Checkpointer::_prepare_threads(Genode::List<Stored_thread_info> &state_infos, Cpu_session_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Thread_info *child_info = nullptr;
	Stored_thread_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.thread_infos().first();
	while(child_info)
	{
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->cpu_thread.cap().local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_thread_info(*child_info);
			state_infos.insert(state_info);
		}

		// Update state_info
		state_info->started              = child_info->cpu_thread.parent_state().started;
		state_info->paused               = child_info->cpu_thread.parent_state().paused;
		state_info->exception_sigh_badge = child_info->cpu_thread.parent_state().exception_sigh.local_name();
		state_info->single_step          = child_info->cpu_thread.parent_state().single_step;
		state_info->affinity             = child_info->cpu_thread.parent_state().affinity;
		state_info->ts = Genode::Cpu_thread_client(child_info->cpu_thread.parent_cap()).state();

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		Stored_thread_info *next_info = state_info->next();

		// Find corresponding child_info
		child_info = child_obj.thread_infos().first();
		while(child_info)
		{
			if(state_info->badge == child_info->cpu_thread.cap().local_name()) break;
			child_info = child_info->next();
		}

		// No corresponding child_info => delete it
		if(!child_info)
		{
			state_infos.remove(state_info);
			Genode::destroy(_state._alloc, state_info);
		}

		state_info = next_info;
	}
}


void Checkpointer::_prepare_contexts(Genode::List<Stored_signal_context_info> &state_infos, Pd_session_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Signal_context_info *child_info = nullptr;
	Stored_signal_context_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.signal_context_infos().first();
	while(child_info)
	{
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->sc_cap.local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_signal_context_info(*child_info);
			state_infos.insert(state_info);
		}

		// No need to update state_info

		child_info = child_info->next();
	}


	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		Stored_signal_context_info *next_info = state_info->next();

		// Find corresponding child_info
		child_info = child_obj.signal_context_infos().first();
		while(child_info)
		{
			if(state_info->badge == child_info->sc_cap.local_name()) break;
			child_info = child_info->next();
		}

		// No corresponding child_info => delete it
		if(!child_info)
		{
			state_infos.remove(state_info);
			Genode::destroy(_state._alloc, state_info);
		}

		state_info = next_info;
	}
}


void Checkpointer::_prepare_sources(Genode::List<Stored_signal_source_info> &state_infos, Pd_session_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Signal_source_info *child_info = nullptr;
	Stored_signal_source_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.signal_source_infos().first();
	while(child_info)
	{
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->cap.local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_signal_source_info(*child_info);
			state_infos.insert(state_info);
		}

		// No need to update state_info

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		Stored_signal_source_info *next_info = state_info->next();

		// Find corresponding child_info
		child_info = child_obj.signal_source_infos().first();
		while(child_info)
		{
			if(state_info->badge == child_info->cap.local_name()) break;
			child_info = child_info->next();
		}

		// No corresponding child_info => delete it
		if(!child_info)
		{
			state_infos.remove(state_info);
			Genode::destroy(_state._alloc, state_info);
		}

		state_info = next_info;
	}
}


void Checkpointer::_prepare_region_map(Stored_region_map_info &state_info, Region_map_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_info=", &state_info, ", child_obj=", &child_obj, ")");

	state_info.badge = child_obj.cap().local_name();

	// Update state_info
	state_info.fault_handler_badge = child_obj.parent_state().fault_handler.local_name();
	state_info.ds_badge = child_obj.parent_state().ds_cap.local_name();

	_prepare_attached_regions(state_info.stored_attached_region_infos, child_obj);
}


void Checkpointer::_update_dataspace_infos(Genode::List<Stored_dataspace_info> &state_infos, Ram_session_component &child_obj,
		Genode::List<Badge_dataspace_info> &visited_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Ram_dataspace_info *child_info = nullptr;
	Stored_dataspace_info *state_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_obj.ram_dataspace_infos().first();
	while(child_info)
	{
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->ds_cap.local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_dataspace_info(*child_info);
			state_info->ds_cap = _state._env.ram().alloc(state_info->size);
			state_infos.insert(state_info);
		}

		// No need to update state_info

		// Remember this RAM dataspace
		Badge_dataspace_info *visited_info =
				new (_alloc) Badge_dataspace_info(state_info->badge, child_info->ds_cap);
		// If child has a managed dataspace (from incremental checkpoint mechanism),
		// then store the capabilities of the designated dataspaces
		if(child_info->mrm_info)
		{
			Designated_dataspace_info *dd_info = child_info->mrm_info->dd_infos.first();
			while(dd_info)
			{
				// Store only dirtied dataspaces
				if(dd_info->attached)
				{
					Managed_dataspace_info *md_info =
							new (_alloc) Managed_dataspace_info(dd_info->ds_cap, dd_info->rel_addr, dd_info->size);
					visited_info->infos.insert(md_info);
				}
				dd_info = dd_info->next();
			}
		}
		visited_infos.insert(visited_info);

		child_info = child_info->next();
	}
}


void Checkpointer::_update_dataspace_infos(Genode::List<Stored_dataspace_info> &state_infos, Region_map_component &child_obj,
		Genode::List<Badge_dataspace_info> &visited_infos, Genode::List<Badge_info> &exclude)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	Badge_info *exclude_info = nullptr;
	Badge_dataspace_info *visited_info = nullptr;
	Stored_dataspace_info *state_info = nullptr;

	Attached_region_info *child_info = child_obj.attached_regions().first();
	for( ; child_info; child_info = child_info->next())
	{
		// If child_info is in exclude list, then proceed with the next child_info
		exclude_info = exclude.first();
		if(exclude_info) exclude_info = exclude_info->find_by_badge(child_info->ds_cap.local_name());
		if(exclude_info) continue;

		// If child_info was seen already, then proceed with the next child_info
		visited_info = visited_infos.first();
		if(visited_info) visited_info = visited_info->find_by_badge(child_info->ds_cap.local_name());
		if(visited_info) continue;

		// child_info is not excluded and was also not seen yet => find corresponding state_info or create a new one
		state_info = state_infos.first();
		if(state_info) state_info = state_info->find_by_badge(child_info->ds_cap.local_name());
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_dataspace_info(*child_info);
			state_info->ds_cap = _state._env.ram().alloc(state_info->size);
			state_infos.insert(state_info);
		}

		// Do not update the ds content of ds_cap here, but later in a separate phase

		// Remember state_info
		Badge_dataspace_info *visited_info =
				new (_alloc) Badge_dataspace_info(state_info->badge, child_info->ds_cap);
		visited_infos.insert(visited_info);
	}
}


void Checkpointer::_update_dataspace_infos(Genode::List<Stored_dataspace_info> &state_infos, Rm_root &child_obj,
		Genode::List<Badge_dataspace_info> &visited_infos, Genode::List<Badge_info> &exclude)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(ds_infos=", &state_infos, ", child_obj=", &child_obj, ")");

	// Iterate through each RM session
	Rm_session_info *rm_session_info = child_obj.rms_infos().first();
	while(rm_session_info)
	{
		// Iterate through each region map
		Region_map_info *region_map_info = rm_session_info->session.region_map_infos().first();
		while(region_map_info)
		{
			// update dataspace list with new dataspaces from this region map
			_update_dataspace_infos(state_infos, region_map_info->region_map, visited_infos, exclude);

			region_map_info = region_map_info->next();
		}

		rm_session_info = rm_session_info->next();
	}
}


void Checkpointer::_delete_old_dataspace_infos(Genode::List<Stored_dataspace_info> &state_infos,
		Genode::List<Badge_dataspace_info> &visited_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Stored_dataspace_info *ds_info = state_infos.first();
	while(ds_info)
	{
		Stored_dataspace_info *next_info = ds_info->next();

		// Determine whether ds_info was visited before
		Badge_dataspace_info *visited_info = visited_infos.first();
		while(visited_info)
		{
			if(ds_info->badge == visited_info->badge) break;
			visited_info = visited_info->next();
		}

		// ds_info was not visited => destroy it
		if(!visited_info)
		{
			state_infos.remove(ds_info);
			_state._env.ram().free(Genode::static_cap_cast<Genode::Ram_dataspace>(ds_info->ds_cap));
			Genode::destroy(_state._alloc, ds_info);
		}

		ds_info = next_info;
	}
}


Genode::List<Checkpointer::Badge_info> Checkpointer::_create_exclude_infos(Region_map_component &address_space,
		Region_map_component &stack_area, Region_map_component &linker_area, Rm_root *rm_root)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::List<Badge_info> exclude_infos;

	exclude_infos.insert(new (_alloc) Badge_info(address_space.parent_state().ds_cap.local_name()));
	exclude_infos.insert(new (_alloc) Badge_info(stack_area.parent_state().ds_cap.local_name()));
	exclude_infos.insert(new (_alloc) Badge_info(linker_area.parent_state().ds_cap.local_name()));

	if(rm_root)
	{
		Rm_session_info *rm_session_info = rm_root->rms_infos().first();
		while(rm_session_info)
		{
			Region_map_info *region_map_info = rm_session_info->session.region_map_infos().first();
			while(region_map_info)
			{
				exclude_infos.insert(new (_alloc) Badge_info(region_map_info->ds_cap.local_name()));
				region_map_info = region_map_info->next();
			}
			rm_session_info = rm_session_info->next();
		}
	}

	return exclude_infos;
}


void Checkpointer::_destroy_exclude_infos(Genode::List<Checkpointer::Badge_info> &infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Badge_info *info = infos.first())
	{
		infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}


void Checkpointer::_detach_designated_dataspaces(Ram_session_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(child_obj=", &child_obj, ")");

	Ram_dataspace_info *rd_info = child_obj.ram_dataspace_infos().first();

	while(rd_info)
	{
		if(rd_info->mrm_info)
		{
			Designated_dataspace_info *dd_info = rd_info->mrm_info->dd_infos.first();
			while(dd_info)
			{
				if(dd_info->attached) dd_info->detach();
				dd_info = dd_info->next();
			}
		}

		rd_info = rd_info->next();
	}
}


void Checkpointer::_checkpoint_dataspaces(Genode::List<Stored_dataspace_info> &state_infos,
		Genode::List<Badge_dataspace_info> &visited_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ")");

	Stored_dataspace_info *state_info = state_infos.first();
	while(state_info)
	{
		Badge_dataspace_info *visited_info = visited_infos.first();
		if(visited_info) visited_info = visited_info->find_by_badge(state_info->badge);
		if(visited_info)
		{
			Managed_dataspace_info *md_info = visited_info->infos.first();
			// Managed dataspace
			if(md_info)
			{
				while(md_info)
				{
					if(!md_info->checkpointed)
						_update_managed_dataspace(*state_info, *md_info);
					md_info = md_info->next();
				}
			}
			// Not managed dataspace
			else
			{
				if(!visited_info->checkpointed)
					_update_normal_dataspace(*state_info, *visited_info);
			}
		}
		else
		{
			Genode::warning("No visited_info found for state_info ", state_info->badge);
		}

		state_info = state_info->next();
	}
}


void Checkpointer::_update_normal_dataspace(Stored_dataspace_info &state_info, Badge_dataspace_info &visited_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_info=", &state_info,
			", child_info=", &visited_info, ")");

	_copy_dataspace_content(visited_info.ds_cap, state_info.ds_cap, state_info.size);
	visited_info.checkpointed = true;
}


void Checkpointer::_update_managed_dataspace(Stored_dataspace_info &state_info, Managed_dataspace_info &md_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_info=", &state_info,
			", managed_ds_info=", &md_info, ")");

	_copy_dataspace_content(md_info.ds_cap, state_info.ds_cap, state_info.size, md_info.rel_addr);
	md_info.checkpointed = true;
}


void Checkpointer::_copy_dataspace_content(Genode::Dataspace_capability src_ds_cap, Genode::Dataspace_capability dst_ds_cap,
		Genode::size_t size, Genode::off_t dst_offset)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(src ", src_ds_cap,
			", dst ", dst_ds_cap, ", size=", Genode::Hex(size), ", dst_offset=", dst_offset, ")");

	char *src = _state._env.rm().attach(src_ds_cap);
	char *dst = _state._env.rm().attach(dst_ds_cap);

	Genode::memcpy(dst + dst_offset, src, size);

	_state._env.rm().detach(dst);
	_state._env.rm().detach(src);
}


Checkpointer::Checkpointer(Genode::Allocator &alloc, Target_child &child, Target_state &state)
:
	_alloc(alloc), _child(child), _state(state)
{
	if(verbose_debug) Genode::log("\033[33m", "Checkpointer", "\033[0m(child=", &_child, ", state=", &_state, ")");
}

Checkpointer::~Checkpointer()
{
	if(verbose_debug) Genode::log("\033[33m", "~Checkpointer", "\033[0m child=", &_child, ", state=", &_state);

	// Delete capability_map_infos
	while(Badge_kcap_info *info = _capability_map_infos.first())
	{
		_capability_map_infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}


void Checkpointer::checkpoint()
{
	/**
	 * Pause child threads
	 *
	 * Create visited_infos
	 *
	 * Prepare cap map list
	 * Prepare state's lists
	 *   Update state list elements corresponding to child list elements
	 *     Create new state list elements, if no state list exists which corresponds to a child list elements
	 *   Delete old list elements, if no corresponding child list element exists
	 *
	 * Detach all dataspaces for COW
	 *
	 * Resume child threads
	 *
	 * Checkpoint dataspaces and set stored and dirty flag
	 *
	 * Destroy visited_infos
	 *
	 */

	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");

	// Pause child
	_child.pause();


	// Prepare cap map list
	_prepare_cap_map_infos(_capability_map_infos);

	// Prepare state lists
	Rm_root *rm_root = _child.rm_root();
	if(rm_root) _prepare_rm_sessions(_state._stored_rm_sessions, *rm_root);
	Log_root *log_root = _child.log_root();
	if(log_root) _prepare_log_sessions(_state._stored_log_sessions, *log_root);
	Timer_root *timer_root = _child.timer_root();
	if(timer_root) _prepare_timer_sessions(_state._stored_timer_sessions, *timer_root);
	_prepare_threads(_state._stored_threads, _child.cpu());
	_prepare_contexts(_state._stored_signal_contexts, _child.pd());
	_prepare_sources(_state._stored_signal_sources, _child.pd());
	_prepare_region_map(_state._stored_address_space, _child.pd().address_space_component());
	_prepare_region_map(_state._stored_stack_area, _child.pd().stack_area_component());
	_prepare_region_map(_state._stored_linker_area, _child.pd().linker_area_component());

	// Create exclude list, which determines whether an attached dataspace is a known region map;
	// if a dataspace is a region map, then it will not be checkpointed, but its dataspaces
	Genode::List<Badge_info> exclude_infos = _create_exclude_infos(_child.pd().address_space_component(),
			_child.pd().stack_area_component(), _child.pd().linker_area_component(), rm_root);
	// Create visited map to mark visited Stored_dataspace_infos; thus, the not visited can be deleted.
	// The second purpose is to store managed dataspaces of the incremental checkpoint mechanism, which
	// is used at checkpoint time
	Genode::List<Badge_dataspace_info> visited_infos;

	// Prepare the dataspace infos
	_update_dataspace_infos(_state._stored_dataspaces, _child.ram(), visited_infos);
	_update_dataspace_infos(_state._stored_dataspaces, _child.pd().address_space_component(), visited_infos, exclude_infos);
	_update_dataspace_infos(_state._stored_dataspaces, _child.pd().stack_area_component(), visited_infos, exclude_infos);
	_update_dataspace_infos(_state._stored_dataspaces, _child.pd().linker_area_component(), visited_infos, exclude_infos);
	if(rm_root) _update_dataspace_infos(_state._stored_dataspaces, *rm_root, visited_infos, exclude_infos);
	_delete_old_dataspace_infos(_state._stored_dataspaces, visited_infos);

	// Detach all designated dataspaces
	_detach_designated_dataspaces(_child.ram());

	// Checkpoint dataspaces
	_checkpoint_dataspaces(_state._stored_dataspaces, visited_infos);

	// Delete exclude_infos
	_destroy_exclude_infos(exclude_infos);
	// Delete visited_infos
	while(Badge_dataspace_info *info = visited_infos.first())
	{
		visited_infos.remove(info);
		Genode::destroy(_alloc, info);
	}

	//Genode::log(_child);
	//Genode::log(_state);
	// Resume child
	//_child.resume();
}
