/*
 * \brief  Checkpointer of Target_state
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "checkpointer.h"

using namespace Rtcr;


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
		}

		// Update state_info
		_prepare_region_maps(state_info->stored_region_map_infos, child_info->session);

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		state_info = state_info->next();
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
		}

		// No need to update state_info

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		state_info = state_info->next();
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
		state_info = state_info->next();
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
		state_info = state_info->next();
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
		if(state_info) state_info = state_info->find_by_badge(child_info->ds_cap.local_name());

		// No corresponding state_info => create it
		if(!state_info)
		{
			state_info = new (_state._alloc) Stored_attached_region_info(*child_info);
		}

		// No need to update state_info

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		state_info = state_info->next();
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
		state_info = state_info->next();
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
		}

		// No need to update state_info

		child_info = child_info->next();
	}


	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		state_info = state_info->next();
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
		}

		// No need to update state_info

		child_info = child_info->next();
	}

	// Delete old state_infos, if the child misses corresponding infos in its list
	state_info = state_infos.first();
	while(state_info)
	{
		state_info = state_info->next();
	}
}


void Checkpointer::_prepare_region_map(Stored_region_map_info &state_info, Region_map_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_info=", &state_info, ", child_obj=", &child_obj, ")");

	// Update state_info
	state_info.fault_handler_badge = child_obj.parent_state().fault_handler.local_name();
	// Address space, stack area, and linker area: Do not need to checkpoint their dataspace_badge and size
	//state_info.dataspace_badge = Genode::Region_map_client(child_obj.parent_cap()).dataspace().local_name();
	//state_info.size = Genode::Dataspace_client((Genode::Region_map_client(child_obj.parent_cap()).dataspace())).size();
	_prepare_attached_regions(state_info.stored_attached_region_infos, child_obj);
}


void Checkpointer::_prepare_dataspaces(Genode::List<Stored_dataspace_info> &state_infos, Genode::List<Badge_dataspace_info> &ds_infos,
		Ram_session_component &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");
}


void Checkpointer::_prepare_dataspaces(Genode::List<Stored_dataspace_info> &state_infos, Genode::List<Badge_dataspace_info> &ds_infos,
		Region_map_component  &child_obj)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_infos=", &state_infos, ", child_obj=", &child_obj, ")");
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


void Checkpointer::_checkpoint_dataspaces()
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
}


void Checkpointer::_update_normal_dataspace(Stored_dataspace_info &state_info, Ram_dataspace_info &child_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_info=", &state_info, ", child_info=", &child_info, ")");
}


void Checkpointer::_update_managed_dataspace(Stored_dataspace_info &state_info, Managed_region_map_info &child_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(state_info=", &state_info, ", child_info=", &child_info, ")");
}


void Checkpointer::_copy_dataspace_content(Genode::Dataspace_capability src_ds_cap, Genode::Dataspace_capability dst_ds_cap,
		Genode::size_t size, Genode::off_t dst_offset)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(src ", src_ds_cap, ", dst ", dst_ds_cap, ", size=",
			size, ", dst_offset=", dst_offset, ")");
}


/*

void Checkpointer::_checkpoint_dataspaces()
{

	// Auxillary list to have child's dataspace capabilities for copying and
	// to know which badges are used to delete old Stored_dataspace_info elements
	Genode::List<Badge_dataspace_info> child_dataspaces;

	// Checkpoint child's allocated dataspaces
	_checkpoint_state(_child.ram(), _state._stored_dataspaces, visited_rpc_objects);

	// Checkpoint dataspaces from PD session's region maps
	_checkpoint_state(_child.pd(), _state._stored_dataspaces, visited_rpc_objects);

	// Delete not visited Stored_dataspace_infos

	// Delete the list of visited RPC objects
	while(Badge_info *info = visited_rpc_objects.first())
	{
		visited_rpc_objects.remove(info);
		Genode::destroy(_state._alloc, info);
	}
}


void Checkpointer::_checkpoint_state(Ram_session_component &ram_session_comp,
		Genode::List<Stored_dataspace_info> &stored_ds_infos, Genode::List<Badge_info> &used_infos)
{
	Ram_dataspace_info *ram_ds_info = ram_session_comp.ram_dataspace_infos().first();
	while(ram_ds_info)
	{
		// Find corresponding Stored_dataspace_info
		Stored_dataspace_info *stored_ds_info = _state._stored_dataspaces.first();
		if(stored_ds_info) stored_ds_info = stored_ds_info->find_by_badge(ram_ds_info->ds_cap.local_name());
		// No corresponding Stored_dataspace_info found => create it
		if(!stored_ds_info)
		{
			stored_ds_info = new (_state._alloc) Stored_dataspace_info();
			Genode::Dataspace_capability copy_ds_cap = _state._env.ram().alloc(ram_ds_info->size);

			stored_ds_info->badge   = ram_ds_info->ds_cap.local_name();
			stored_ds_info->kcap    = _capability_map_infos.first()->find_by_badge(stored_ds_info->badge)->kcap;
			stored_ds_info->ds_cap  = copy_ds_cap;
			stored_ds_info->size    = ram_ds_info->size;
			stored_ds_info->cached  = ram_ds_info->cached;
			stored_ds_info->managed = ram_ds_info->mrm_info; // implicit conversion from pointer to bool

			_state._stored_dataspaces.insert(stored_ds_info);
		}

		// Mark stored_dataspace_info as visited
		used_infos.insert(new (_state._alloc) Badge_info(stored_ds_info->badge));

		// Next RAM dataspace
		ram_ds_info = ram_ds_info->next();
	}
}


void Checkpointer::_update_normal_dataspace(Ram_dataspace_info &ram_ds_info, Stored_dataspace_info &stored_ds_info)
{
	_copy_dataspace_content(ram_ds_info.ds_cap, stored_ds_info.ds_cap, ram_ds_info.size);
}


void Checkpointer::_update_managed_dataspace(Managed_region_map_info &mrm_info, Stored_dataspace_info &stored_ds_info)
{
	Designated_dataspace_info *dd_info = mrm_info.dd_infos.first();
	while(dd_info)
	{
		if(dd_info->dirty)
		{

		}

		dd_info = dd_info->next();
	}
}
*/

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
	 * Prepare state's lists
	 *   Update state list elements corresponding to child list elements
	 *     Create new state list elements, if no corresponding child list elements exists
	 *   Delete old list elements, if no corresponding child list element exists
	 *
	 * Detach all dataspaces for COW
	 *
	 * Resume child threads
	 *
	 * Checkpoint dataspaces and set stored and dirty flag
	 *
	 * Destroy visited_infos
	 */

	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");

	// Pause child
	_child.pause();

	// Create visited map
	Genode::List<Badge_dataspace_info> visited_infos;

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

	// Detach all designated dataspaces
	_detach_designated_dataspaces(_child.ram());

	// Checkpoint dataspaces
	_prepare_dataspaces(_state._stored_dataspaces, visited_infos, _child.ram());
	_prepare_dataspaces(_state._stored_dataspaces, visited_infos, _child.pd().address_space_component());
	_prepare_dataspaces(_state._stored_dataspaces, visited_infos, _child.pd().stack_area_component());
	_prepare_dataspaces(_state._stored_dataspaces, visited_infos, _child.pd().linker_area_component());

	// Resume child
	//_child.resume();

	// Delete visited_infos
	while(Badge_dataspace_info *info = visited_infos.first())
	{
		visited_infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}
