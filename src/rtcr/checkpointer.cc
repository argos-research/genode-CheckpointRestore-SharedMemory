/*
 * \brief  Checkpointer of Target_state
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "checkpointer.h"

using namespace Rtcr;


Genode::List<Stored_rm_session_info> Checkpointer::_checkpoint_state(Rm_root &rm_root)
{
	Genode::List<Stored_rm_session_info> state_infos;

	Rm_session_info *child_info = rm_root.rms_infos().first();
	while(child_info)
	{
		Stored_rm_session_info *state_info = new (_state._alloc) Stored_rm_session_info();

		state_info->badge = child_info->session.cap().local_name();
		state_info->kcap  = _capability_map_infos.first()->find_by_badge(state_info->badge)->kcap;
		state_info->args  = child_info->args;
		state_info->stored_region_map_infos = _checkpoint_state(child_info->session);

		state_infos.insert(state_info);

		child_info = child_info->next();
	}

	return state_infos;
}


Genode::List<Stored_region_map_info> Checkpointer::_checkpoint_state(Rm_session_component &rm_session_comp)
{
	Genode::List<Stored_region_map_info> state_infos;

	Region_map_info *child_info = rm_session_comp.region_map_infos().first();
	while(child_info)
	{
		Stored_region_map_info *state_info = new (_state._alloc) Stored_region_map_info();

		state_info->badge = child_info->region_map.cap().local_name();
		state_info->kcap  = _capability_map_infos.first()->find_by_badge(state_info->badge)->kcap;
		state_info->size  = child_info->size;
		state_info->fault_handler_badge = child_info->region_map.parent_state().fault_handler.local_name();
		state_info->dataspace_badge = Genode::Region_map_client(child_info->region_map.parent_cap()).dataspace().local_name();
		state_info->stored_attached_region_infos = _checkpoint_state(child_info->region_map);

		child_info = child_info->next();
	}

	return state_infos;
}


Genode::List<Stored_attached_region_info> Checkpointer::_checkpoint_state(Region_map_component &region_map_comp)
{
	Genode::List<Stored_attached_region_info> state_infos;

	Attached_region_info *child_info = region_map_comp.attached_regions().first();
	while(child_info)
	{
		Stored_attached_region_info *state_info = new (_state._alloc) Stored_attached_region_info();

		state_info->badge      = child_info->ds_cap.local_name();
		state_info->size       = child_info->size;
		state_info->offset     = child_info->offset;
		state_info->rel_addr   = child_info->rel_addr;
		state_info->executable = child_info->executable;

		child_info = child_info->next();
	}

	return state_infos;
}


Genode::List<Stored_log_session_info> Checkpointer::_checkpoint_state(Log_root &log_root)
{
	Genode::List<Stored_log_session_info> state_infos;

	Log_session_info *child_info = log_root.session_infos().first();
	while(child_info)
	{
		Stored_log_session_info *state_info = new (_state._alloc) Stored_log_session_info();

		state_info->badge = child_info->session.cap().local_name();
		state_info->kcap  = _capability_map_infos.first()->find_by_badge(state_info->badge)->kcap;
		state_info->args  = child_info->args;

		child_info = child_info->next();
	}

	return state_infos;
}


Genode::List<Stored_timer_session_info> Checkpointer::_checkpoint_state(Timer_root &timer_root)
{
	Genode::List<Stored_timer_session_info> state_infos;

	Timer_session_info *child_info = timer_root.session_infos().first();
	while(child_info)
	{
		Stored_timer_session_info *state_info = new (_state._alloc) Stored_timer_session_info();

		state_info->badge      = child_info->session.cap().local_name();
		state_info->kcap       = _capability_map_infos.first()->find_by_badge(state_info->badge)->kcap;
		state_info->args       = child_info->args;
		state_info->sigh_badge = child_info->session.parent_state().sigh.local_name();
		state_info->timeout    = child_info->session.parent_state().timeout;
		state_info->periodic   = child_info->session.parent_state().periodic;

		child_info = child_info->next();
	}

	return state_infos;
}


Genode::List<Stored_signal_context_info> Checkpointer::_checkpoint_signal_context(Pd_session_component &pd_session_comp)
{
	Genode::List<Stored_signal_context_info> state_infos;

	Signal_context_info *child_info = pd_session_comp.signal_context_infos().first();
	while(child_info)
	{
		Stored_signal_context_info *state_info = new (_state._alloc) Stored_signal_context_info();

		state_info->badge = child_info->sc_cap.local_name();
		state_info->kcap  = _capability_map_infos.first()->find_by_badge(state_info->badge)->kcap;
		state_info->signal_source_badge = child_info->ss_cap.local_name();
		state_info->imprint = child_info->imprint;

		child_info = child_info->next();
	}

	return state_infos;
}


Genode::List<Stored_signal_source_info> Checkpointer::_checkpoint_signal_source(Pd_session_component &pd_session_comp)
{
	Genode::List<Stored_signal_source_info> state_infos;

	Signal_source_info *child_info = pd_session_comp.signal_source_infos().first();
	while(child_info)
	{
		Stored_signal_source_info *state_info = new (_state._alloc) Stored_signal_source_info();

		state_info->badge = child_info->cap.local_name();
		state_info->kcap  = _capability_map_infos.first()->find_by_badge(state_info->badge)->kcap;

		child_info = child_info->next();
	}

	return state_infos;
}


Genode::List<Stored_thread_info> Checkpointer::_checkpoint_state(Cpu_session_component &cpu_session_comp)
{
	Genode::List<Stored_thread_info> state_infos;

	Thread_info *child_info = cpu_session_comp.thread_infos().first();
	while(child_info)
	{
		Stored_thread_info *state_info = new (_state._alloc) Stored_thread_info();

		state_info->badge       = child_info->cpu_thread.cap().local_name();
		state_info->kcap        = _capability_map_infos.first()->find_by_badge(state_info->badge)->kcap;
		state_info->started     = child_info->cpu_thread.parent_state().started;
		state_info->paused      = child_info->cpu_thread.parent_state().paused;
		state_info->exception_sigh_badge = child_info->cpu_thread.parent_state().exception_sigh.local_name();
		state_info->single_step = child_info->cpu_thread.parent_state().single_step;
		state_info->name        = child_info->name;
		state_info->affinity    = child_info->affinity;
		state_info->weight      = child_info->weight;
		state_info->utcb        = child_info->utcb;
		state_info->ts          = Genode::Cpu_thread_client(child_info->cpu_thread.parent_cap()).state();

		child_info = child_info->next();
	}

	return state_infos;
}


Stored_region_map_info Checkpointer::_checkpoint_region_map(Region_map_component &region_map_comp)
{
	Stored_region_map_info state_info;

	state_info.badge = region_map_comp.cap().local_name();
	state_info.kcap  = _capability_map_infos.first()->find_by_badge(state_info.badge)->kcap;
	state_info.size  = 0; // Not important, because this region map will be created by the PD session
	state_info.fault_handler_badge = region_map_comp.parent_state().fault_handler.local_name();
	state_info.dataspace_badge     = Genode::Region_map_client(region_map_comp.parent_cap()).dataspace().local_name();
	state_info.stored_attached_region_infos = _checkpoint_state(region_map_comp);

	return state_info;
}


Checkpointer::Checkpointer(Target_child &child, Target_state &state)
:
	_child(child), _state(state), _capability_map_infos()
{ }


void Checkpointer::checkpoint()
{
	Rm_root *rm_root = _child.rm_root();
	if(rm_root) _state._stored_rm_sessions = _checkpoint_state(*rm_root);

	Log_root *log_root = _child.log_root();
	if(log_root) _state._stored_log_sessions = _checkpoint_state(*log_root);

	Timer_root *timer_root = _child.timer_root();
	if(timer_root) _state._stored_timer_sessions = _checkpoint_state(*timer_root);

	_state._stored_threads = _checkpoint_state(_child.cpu());
}
