/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "restorer.h"
#include "util/sort.h"

using namespace Rtcr;


void Restorer::_identify_recreate_pd_sessions(Genode::List<Pd_session_component> &pd_sessions,
		Genode::List<Stored_pd_session_info> &stored_pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only one PD session by now (created by bootstrap)
	Pd_session_component *bootstrapped_pd_session = pd_sessions.first();

	// Error
	if(!bootstrapped_pd_session)
	{
		Genode::error("There is no bootstrapped PD session");
		throw Genode::Exception();
	}
	if(bootstrapped_pd_session->next()) Genode::warning("There are more than 1 bootstrapped PD sessions");

	Stored_pd_session_info *stored_pd_session = stored_pd_sessions.first();
	while(stored_pd_session)
	{
		Pd_session_component *pd_session = nullptr;
		if(stored_pd_session->bootstrapped)
		{
			// Identify bootstrapped PD session
			pd_session = bootstrapped_pd_session;
		}
		else
		{
			// Create PD session
			Genode::Rpc_in_buffer<160> creation_args(stored_pd_session->creation_args.string());
			Genode::Session_capability pd_session_cap = _child.custom_services().pd_root->session(creation_args, Genode::Affinity());
			pd_session = _child.custom_services().pd_root->session_infos().first();
			if(pd_session) pd_session = pd_session->find_by_badge(pd_session_cap.local_name());
			if(!pd_session)
			{
				Genode::error("Could not find newly created PD session for ", pd_session_cap);
				throw Genode::Exception();
			}
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_pd_session->badge, pd_session->cap()));

		_identify_recreate_signal_sources(*pd_session, stored_pd_session->stored_source_infos);
		_identify_recreate_signal_contexts(*pd_session, stored_pd_session->stored_context_infos);
		// XXX postpone native caps creation, because it needs capabilities from CPU thread

		stored_pd_session = stored_pd_session->next();
	}

}


void Restorer::_identify_recreate_signal_sources(Pd_session_component &pd_session,
		Genode::List<Stored_signal_source_info> &stored_signal_sources)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only one signal source by now (created by bootstrap)
	Signal_source_info *bootstrapped_info = pd_session.parent_state().signal_sources.first();
	if(!bootstrapped_info)
	{
		Genode::error("There is no bootstrapped Signal source info");
		throw Genode::Exception();
	}
	if(bootstrapped_info->next()) Genode::warning("There are more than 1 bootstrapped PD sessions");

	Signal_source_info *signal_source = nullptr;
	Stored_signal_source_info *stored_signal_source = stored_signal_sources.first();
	while(stored_signal_source)
	{
		if(stored_signal_source->bootstrapped)
		{
			// Identify bootstrapped signal source
			signal_source = bootstrapped_info;
		}
		else
		{
			// Create signal source
			Genode::Capability<Genode::Signal_source> cap = pd_session.alloc_signal_source();
			signal_source = pd_session.parent_state().signal_sources.first();
			if(signal_source) signal_source = signal_source->find_by_badge(cap.local_name());
			if(!signal_source)
			{
				Genode::error("Could not find newly created signal source for ", cap);
				throw Genode::Exception();
			}
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_signal_source->badge, signal_source->cap));

		stored_signal_source = stored_signal_source->next();
	}
}


void Restorer::_identify_recreate_signal_contexts(Pd_session_component &pd_session,
		Genode::List<Stored_signal_context_info> &stored_signal_contexts)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	if(pd_session.parent_state().signal_contexts.first()) Genode::warning("There are bootstrapped signal contexts, expected 0");

	Signal_context_info *signal_context = nullptr;
	Stored_signal_context_info *stored_signal_context = stored_signal_contexts.first();
	while(stored_signal_context)
	{
		// Nothing to identify

		// Create signal context
		Genode::Capability<Genode::Signal_source> ss_cap;
		if(stored_signal_context->signal_source_badge != 0)
		{
			Ckpt_resto_badge_info *info = _ckpt_to_resto_infos.first();
			if(info) info = info->find_by_ckpt_badge(stored_signal_context->signal_source_badge);
			ss_cap = Genode::reinterpret_cap_cast<Genode::Signal_source>(info->resto_cap);
		}

		Genode::Capability<Genode::Signal_context> cap = pd_session.alloc_context(ss_cap, stored_signal_context->imprint);
		signal_context = pd_session.parent_state().signal_contexts.first();
		if(signal_context) signal_context = signal_context->find_by_sc_badge(cap.local_name());
		if(!signal_context)
		{
			Genode::error("Could not find newly created signal context for ", cap);
			throw Genode::Exception();
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_signal_context->badge, signal_context->cap));

		stored_signal_context = stored_signal_context->next();
	}
}


void Restorer::_identify_recreate_ram_sessions( Genode::List<Ram_session_component> &ram_sessions,
		Genode::List<Stored_ram_session_info> &stored_ram_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only one RAM session by now (created by bootstrap)
	Ram_session_component *bootstrapped_ram_session = ram_sessions.first();

	// Error
	if(!bootstrapped_ram_session)
	{
		Genode::error("There is no bootstrapped RAM session");
		throw Genode::Exception();
	}
	if(bootstrapped_ram_session->next()) Genode::warning("There are more than 1 bootstrapped RAM sessions");

	Stored_ram_session_info *stored_ram_session = stored_ram_sessions.first();
	while(stored_ram_session)
	{
		Ram_session_component *ram_session = nullptr;
		if(stored_ram_session->bootstrapped)
		{
			// Identify bootstrapped RAM session
			ram_session = bootstrapped_ram_session;
		}
		else
		{
			// Create RAM session
			Genode::Rpc_in_buffer<160> creation_args(stored_ram_session->creation_args.string());
			Genode::Session_capability ram_session_cap = _child.custom_services().ram_root->session(creation_args, Genode::Affinity());
			ram_session = _child.custom_services().ram_root->session_infos().first();
			if(ram_session) ram_session = ram_session->find_by_badge(ram_session_cap.local_name());
			if(!ram_session)
			{
				Genode::error("Could not find newly created RAM session for ", ram_session_cap);
				throw Genode::Exception();
			}
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_ram_session->badge, ram_session->cap()));

		_identify_recreate_ram_dataspaces(*ram_session, stored_ram_session->stored_ramds_infos);

		stored_ram_session = stored_ram_session->next();
	}
}


void Restorer::_identify_recreate_ram_dataspaces(Ram_session_component &ram_session,
		Genode::List<Stored_ram_dataspace_info> &stored_ram_dataspaces)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only at least 1 RAM dataspace by now (created by bootstrap)
	Genode::List<Ckpt_resto_badge_info> bootstrapped_ram_dataspaces =
			_identify_ram_dataspaces(ram_session.parent_state().ram_dataspaces, stored_ram_dataspaces);
	if(!bootstrapped_ram_dataspaces.first())
	{
		Genode::error("There is no bootstrapped RAM dataspace");
		throw Genode::Exception();
	}

	Ram_dataspace_info *ramds = nullptr;
	Stored_ram_dataspace_info *stored_ramds = stored_ram_dataspaces.first();
	while(stored_ramds)
	{
		if(stored_ramds->bootstrapped)
		{
			// Identify
			Ckpt_resto_badge_info *cr_info = bootstrapped_ram_dataspaces.first();
			cr_info = cr_info->find_by_ckpt_badge(stored_ramds->badge);
			ramds = ram_session.parent_state().ram_dataspaces.first();
			if(ramds) ramds = ramds->find_by_badge(cr_info->resto_cap.local_name());
			if(!ramds)
			{
				Genode::error("Could not find bootstrapped RAM dataspace for badge ", stored_ramds->badge);
				throw Genode::Exception();
			}
		}
		else
		{
			// Recreate
			Genode::Ram_dataspace_capability cap = ram_session.alloc(stored_ramds->size, stored_ramds->cached);
			ramds = ram_session.parent_state().ram_dataspaces.first();
			if(ramds) ramds = ramds->find_by_badge(cap.local_name());
			if(!ramds)
			{
				Genode::error("Could not find newly created RAM dataspace for ", cap);
				throw Genode::Exception();
			}
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_ramds->badge, ramds->cap));

		stored_ramds = stored_ramds->next();
	}

	// Clean up
	while(Ckpt_resto_badge_info *info = bootstrapped_ram_dataspaces.first())
	{
		bootstrapped_ram_dataspaces.remove(info);
		Genode::destroy(_alloc, info);
	}
}

Genode::List<Ckpt_resto_badge_info> Restorer::_identify_ram_dataspaces(
		Genode::List<Ram_dataspace_info> &ram_dataspaces, Genode::List<Stored_ram_dataspace_info> &stored_ram_dataspaces)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_ram_dataspace_info *stored_ramds = nullptr;
	Ram_dataspace_info *ramds = nullptr;
	Genode::size_t size_stored_array = 0;
	Genode::size_t size_array = 0;

	// Find out array sizes
	stored_ramds = stored_ram_dataspaces.first();
	while(stored_ramds)
	{
		if(stored_ramds->bootstrapped) size_stored_array++;
		stored_ramds = stored_ramds->next();
	}
	ramds = ram_dataspaces.first();
	while(ramds)
	{
		if(ramds->bootstrapped) size_array++;
		ramds = ramds->next();
	}

	if(size_stored_array != size_array)
	{
		Genode::warning("Size of bootstrapped stored-ramdses (", size_stored_array,
				") is not equal to bootstrapped ramdses (", size_array, ")");
	}

	// Create arrays
	Genode::size_t stored_array[size_stored_array];
	Genode::size_t array[size_array];

	// Fill arrays
	stored_ramds = stored_ram_dataspaces.first();
	unsigned i = 0;
	while(stored_ramds)
	{
		if(stored_ramds->bootstrapped) stored_array[i] = stored_ramds->timestamp;
		stored_ramds = stored_ramds->next();
		++i;
	}
	ramds = ram_dataspaces.first();
	i = 0;
	while(ramds)
	{
		if(ramds->bootstrapped) array[i] = ramds->timestamp();
		ramds = ramds->next();
		++i;
	}

	// Sort arrays
	merge_sort(stored_array, size_stored_array);
	merge_sort(array, size_array);

	// Create Result
	Genode::List<Ckpt_resto_badge_info> result;
	for(unsigned i = 0; i < size_stored_array; ++i)
	{
		stored_ramds = stored_ram_dataspaces.first()->find_by_timestamp(stored_array[i]);
		ramds = ram_dataspaces.first()->find_by_timestamp(array[i]);

		result.insert(new (_alloc) Ckpt_resto_badge_info(stored_ramds->badge, ramds->cap));
	}

	return result;
}


void Restorer::_identify_recreate_cpu_sessions(Genode::List<Cpu_session_component> &cpu_sessions,
		Genode::List<Stored_cpu_session_info> &stored_cpu_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only one CPU session by now (created by bootstrap)
	Cpu_session_component *bootstrapped_cpu_session = cpu_sessions.first();

	// Error
	if(!bootstrapped_cpu_session)
	{
		Genode::error("There is no bootstrapped CPU session");
		throw Genode::Exception();
	}
	if(bootstrapped_cpu_session->next()) Genode::warning("There are more than 1 bootstrapped CPU sessions");

	Cpu_session_component *cpu_session = nullptr;
	Stored_cpu_session_info *stored_cpu_session = stored_cpu_sessions.first();
	while(stored_cpu_session)
	{
		if(stored_cpu_session->bootstrapped)
		{
			// Identify
			cpu_session = bootstrapped_cpu_session;

		}
		else
		{
			// Recreate
			Genode::Rpc_in_buffer<160> creation_args(stored_cpu_session->creation_args.string());
			Genode::Session_capability cpu_session_cap = _child.custom_services().cpu_root->session(creation_args, Genode::Affinity());
			cpu_session = _child.custom_services().cpu_root->session_infos().first();
			if(cpu_session) cpu_session = cpu_session->find_by_badge(cpu_session_cap.local_name());
			if(!cpu_session)
			{
				Genode::error("Could not find newly created RAM session for ", cpu_session_cap);
				throw Genode::Exception();
			}
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_cpu_session->badge, cpu_session->cap()));

		//TODO
		//_identify_recreate_cpu_threads(*cpu_session, stored_cpu_session->stored_cpu_thread_infos);


		stored_cpu_session = stored_cpu_session->next();
	}
}


void Restorer::_identify_recreate_rm_sessions(Genode::List<Rm_session_component> &rm_sessions,
		Genode::List<Stored_rm_session_info> &stored_rm_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only no RM session created by now (created by bootstrap)
	if(rm_sessions.first()) Genode::warning("There are already created RM sessions");

	Rm_session_component *rm_session = nullptr;
	Stored_rm_session_info *stored_rm_session = stored_rm_sessions.first();
	while(stored_rm_session)
	{
		// Nothing to identify

		// Recreate
		Genode::Rpc_in_buffer<160> creation_args(stored_rm_session->creation_args.string());
		Genode::Session_capability rm_session_cap = _child.custom_services().rm_root->session(creation_args, Genode::Affinity());
		rm_session = _child.custom_services().rm_root->session_infos().first();
		if(rm_session) rm_session = rm_session->find_by_badge(rm_session_cap.local_name());
		if(!rm_session)
		{
			Genode::error("Could not find newly created RM session for ", rm_session_cap);
			throw Genode::Exception();
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_rm_session->badge, rm_session->cap()));

		//TODO
		//_identify_recreate_region_maps(*rm_session, stored_rm_session->stored_region_map_infos);

		stored_rm_session = stored_rm_session->next();
	}
}


void Restorer::_identify_recreate_log_sessions(Genode::List<Log_session_component> &log_sessions,
		Genode::List<Stored_log_session_info> &stored_log_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only no LOG session created by now (created by bootstrap)
	if(log_sessions.first()) Genode::warning("There are already created LOG sessions");

	Log_session_component *log_session = nullptr;
	Stored_log_session_info *stored_log_session = stored_log_sessions.first();
	while(stored_log_session)
	{
		// Nothing to identify

		// Recreate
		Genode::Rpc_in_buffer<160> creation_args(stored_log_session->creation_args.string());
		Genode::Session_capability log_session_cap = _child.custom_services().log_root->session(creation_args, Genode::Affinity());
		log_session = _child.custom_services().log_root->session_infos().first();
		if(log_session) log_session = log_session->find_by_badge(log_session_cap.local_name());
		if(!log_session)
		{
			Genode::error("Could not find newly created LOG session for ", log_session_cap);
			throw Genode::Exception();
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_log_session->badge, log_session->cap()));

		stored_log_session = stored_log_session->next();
	}
}


void Restorer::_identify_recreate_timer_sessions(Genode::List<Timer_session_component> &timer_sessions,
		Genode::List<Stored_timer_session_info> &stored_timer_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only no RM session created by now (created by bootstrap)
	if(timer_sessions.first()) Genode::warning("There are already created Timer sessions");

	Timer_session_component *timer_session = nullptr;
	Stored_timer_session_info *stored_timer_session = stored_timer_sessions.first();
	while(stored_timer_session)
	{
		// Nothing to identify

		// Recreate
		Genode::Rpc_in_buffer<160> creation_args(stored_timer_session->creation_args.string());
		Genode::Session_capability timer_session_cap = _child.custom_services().timer_root->session(creation_args, Genode::Affinity());
		timer_session = _child.custom_services().timer_root->session_infos().first();
		if(timer_session) timer_session = timer_session->find_by_badge(timer_session_cap.local_name());
		if(!timer_session)
		{
			Genode::error("Could not find newly created Timer session for ", timer_session_cap);
			throw Genode::Exception();
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_timer_session->badge, timer_session->cap()));

		stored_timer_session = stored_timer_session->next();
	}
}





















void Restorer::_restore_state_pd_sessions(Genode::List<Pd_session_component> &pd_sessions,
		Genode::List<Stored_pd_session_info> &stored_pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Pd_session_component *pd_session = pd_sessions.first();
	while(pd_session)
	{
		// Find corresponding stored PD session
		Stored_pd_session_info &stored_pd_session = _find_stored_object(*pd_session, stored_pd_sessions);

		// Restore state
		pd_session->parent_state().upgrade_args = stored_pd_session.upgrade_args;

		// Upgrade session
		Genode::size_t ram_quota = Genode::Arg_string::find_arg(pd_session->parent_state().upgrade_args.string(), "ram_quota").ulong_value(0);
		if(ram_quota != 0) _state._env.parent().upgrade(pd_session->parent_cap(), pd_session->parent_state().upgrade_args.string());


		//_restore_state_region_map(address_space);
		//_restore_state_region_map(stack_area);
		//_restore_state_region_map(linker_area);

		pd_session = pd_session->next();
	}
}


Restorer::Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state)
: _alloc(alloc), _child(child), _state(state) { }


Restorer::~Restorer()
{

}


void Restorer::restore()
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m()");

	Genode::log("Before: \n", _child);

	// Identify or create RPC objects (caution: PD <- CPU thread <- Native cap ( "<-" means requires))
	//   Make a mapping of old badges to new badges
	_identify_recreate_pd_sessions(_child.custom_services().pd_root->session_infos(), _state._stored_pd_sessions);
	_identify_recreate_ram_sessions(_child.custom_services().ram_root->session_infos(), _state._stored_ram_sessions);
	_identify_recreate_cpu_sessions(_child.custom_services().cpu_root->session_infos(), _state._stored_cpu_sessions);
	if(_state._stored_rm_sessions.first())
	{
		_child.custom_services().find("RM"); // Implicitly creates root object
		_identify_recreate_rm_sessions(_child.custom_services().rm_root->session_infos(), _state._stored_rm_sessions);
	}
	if(_state._stored_log_sessions.first())
	{
		_child.custom_services().find("LOG"); // Implicitly creates root object
		_identify_recreate_log_sessions(_child.custom_services().log_root->session_infos(), _state._stored_log_sessions);
	}
	if(_state._stored_timer_sessions.first())
	{
		_child.custom_services().find("Timer"); // Implicitly creates root object
		_identify_recreate_timer_sessions(_child.custom_services().timer_root->session_infos(), _state._stored_timer_sessions);
	}


	if(verbose_debug)
	{
		Genode::log("Ckpt to resto infos:");
		Ckpt_resto_badge_info *info = _ckpt_to_resto_infos.first();
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Restore state of all objects using mapping of old badges to new badges
	//   Also make a mapping of memory to restore
	_restore_state_pd_sessions(_child.custom_services().pd_root->session_infos(), _state._stored_pd_sessions);

	// Replace old badges with new in capability map
	//   copy memory content from checkpointed dataspace which contains the cap map
	//   mark mapping from memory to restore as restored
	// Insert capabilities of all objects into capability space

	Genode::log("After: \n", _child);

}
