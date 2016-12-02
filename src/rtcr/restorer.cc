/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "restorer.h"
#include "util/sort.h"
#include "util/debug.h"
#include <base/internal/cap_map.h>
#include <base/internal/cap_alloc.h>

using namespace Rtcr;

template<typename T>
void Restorer::_destroy_list(Genode::List<T> &list)
{
	while(T *elem = list.first())
	{
		list.remove(elem);
		Genode::destroy(_alloc, elem);
	}
}
template void Restorer::_destroy_list(Genode::List<Ckpt_resto_badge_info> &list);
template void Restorer::_destroy_list(Genode::List<Orig_copy_resto_info> &list);
template void Restorer::_destroy_list(Genode::List<Ref_badge> &list);
template void Restorer::_destroy_list(Genode::List<Cap_kcap_info> &list);


Genode::List<Ref_badge> Restorer::_create_region_map_dataspaces(
		Genode::List<Stored_pd_session_info> &stored_pd_sessions, Genode::List<Stored_rm_session_info> &stored_rm_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Genode::List<Ref_badge> result;

	// Get ds_badges from PD sessions
	Stored_pd_session_info *stored_pd_session = stored_pd_sessions.first();
	while(stored_pd_session)
	{
		result.insert(new (_alloc) Ref_badge(stored_pd_session->stored_address_space.ds_badge));
		result.insert(new (_alloc) Ref_badge(stored_pd_session->stored_stack_area.ds_badge));
		result.insert(new (_alloc) Ref_badge(stored_pd_session->stored_linker_area.ds_badge));

		stored_pd_session = stored_pd_session->next();
	}

	// Get ds_badges from RM sessions
	Stored_rm_session_info *stored_rm_session = stored_rm_sessions.first();
	while(stored_rm_session)
	{
		Stored_region_map_info *stored_region_map = stored_rm_session->stored_region_map_infos.first();
		while(stored_region_map)
		{
			result.insert(new (_alloc) Ref_badge(stored_region_map->ds_badge));

			stored_region_map = stored_region_map->next();
		}

		stored_rm_session = stored_rm_session->next();
	}


	return result;
}


void Restorer::_identify_recreate_pd_sessions(Pd_root &pd_root, Genode::List<Stored_pd_session_info> &stored_pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only one PD session by now (created by bootstrap)
	Pd_session_component *bootstrapped_pd_session = pd_root.session_infos().first();

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
			// Recreate PD session
			Genode::Rpc_in_buffer<160> creation_args(stored_pd_session->creation_args.string());
			Genode::Session_capability pd_session_cap = pd_root.session(creation_args, Genode::Affinity());
			pd_session = _child.custom_services().pd_root->session_infos().first();
			if(pd_session) pd_session = pd_session->find_by_badge(pd_session_cap.local_name());
			if(!pd_session)
			{
				Genode::error("Could not find newly created PD session for ", pd_session_cap);
				throw Genode::Exception();
			}
			// Store kcap for the badge of the newly created RPC object
			_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_pd_session->kcap, pd_session->cap()));
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_pd_session->badge, pd_session->cap()));

		_identify_recreate_signal_sources(*pd_session, stored_pd_session->stored_source_infos);
		_identify_recreate_signal_contexts(*pd_session, stored_pd_session->stored_context_infos);
		// XXX postpone native caps creation, because it needs capabilities from CPU thread

		// Insert region map cap and region map's dataspace cap into _ckpt_to_resto_infos for all three region maps of a PD session
		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(
				stored_pd_session->stored_address_space.badge, pd_session->address_space_component().cap()));
		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(
				stored_pd_session->stored_address_space.ds_badge, pd_session->address_space_component().parent_state().ds_cap));

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(
				stored_pd_session->stored_stack_area.badge, pd_session->stack_area_component().cap()));
		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(
				stored_pd_session->stored_stack_area.ds_badge, pd_session->stack_area_component().parent_state().ds_cap));

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(
				stored_pd_session->stored_linker_area.badge, pd_session->linker_area_component().cap()));
		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(
				stored_pd_session->stored_linker_area.ds_badge, pd_session->linker_area_component().parent_state().ds_cap));

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
			// Store kcap for the badge of the newly created RPC object
			_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_signal_source->kcap, signal_source->cap));
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
		if(signal_context) signal_context = signal_context->find_by_badge(cap.local_name());
		if(!signal_context)
		{
			Genode::error("Could not find newly created signal context for ", cap);
			throw Genode::Exception();
		}
		// Store kcap for the badge of the newly created RPC object
		_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_signal_context->kcap, signal_context->cap));

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_signal_context->badge, signal_context->cap));

		stored_signal_context = stored_signal_context->next();
	}
}


void Restorer::_identify_recreate_ram_sessions(Ram_root &ram_root, Genode::List<Stored_ram_session_info> &stored_ram_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only one RAM session by now (created by bootstrap)
	Ram_session_component *bootstrapped_ram_session = ram_root.session_infos().first();

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
			Genode::Session_capability ram_session_cap = ram_root.session(creation_args, Genode::Affinity());
			ram_session = _child.custom_services().ram_root->session_infos().first();
			if(ram_session) ram_session = ram_session->find_by_badge(ram_session_cap.local_name());
			if(!ram_session)
			{
				Genode::error("Could not find newly created RAM session for ", ram_session_cap);
				throw Genode::Exception();
			}
			// Store kcap for the badge of the newly created RPC object
			_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_ram_session->kcap, ram_session->cap()));
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

	// There shall be at least 1 RAM dataspace by now (created by bootstrap)
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
			// Store kcap for the badge of the newly created RPC object
			_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_ramds->kcap, ramds->cap));
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
		Genode::warning("Size of bootstrapped stored-ram_dataspaces (", size_stored_array,
				") is not equal to bootstrapped ram_dataspaces (", size_array, ")");
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

	// Fill result
	Genode::List<Ckpt_resto_badge_info> result;
	for(unsigned i = 0; i < size_stored_array; ++i)
	{
		stored_ramds = stored_ram_dataspaces.first()->find_by_timestamp(stored_array[i]);
		ramds = ram_dataspaces.first()->find_by_timestamp(array[i]);

		result.insert(new (_alloc) Ckpt_resto_badge_info(stored_ramds->badge, ramds->cap));
	}

	return result;
}


void Restorer::_identify_recreate_cpu_sessions(Cpu_root &cpu_root, Genode::List<Stored_cpu_session_info> &stored_cpu_sessions,
		Genode::List<Pd_session_component> &pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only one CPU session by now (created by bootstrap)
	Cpu_session_component *bootstrapped_cpu_session = cpu_root.session_infos().first();

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
			Genode::Session_capability cpu_session_cap = cpu_root.session(creation_args, Genode::Affinity());
			cpu_session = _child.custom_services().cpu_root->session_infos().first();
			if(cpu_session) cpu_session = cpu_session->find_by_badge(cpu_session_cap.local_name());
			if(!cpu_session)
			{
				Genode::error("Could not find newly created RAM session for ", cpu_session_cap);
				throw Genode::Exception();
			}
			// Store kcap for the badge of the newly created RPC object
			_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_cpu_session->kcap, cpu_session->cap()));
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_cpu_session->badge, cpu_session->cap()));

		_identify_recreate_cpu_threads(*cpu_session, stored_cpu_session->stored_cpu_thread_infos, pd_sessions);


		stored_cpu_session = stored_cpu_session->next();
	}
}


void Restorer::_identify_recreate_cpu_threads(Cpu_session_component &cpu_session, Genode::List<Stored_cpu_thread_info> &stored_cpu_threads,
		Genode::List<Pd_session_component> &pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only 3 CPU threads by now (created by bootstrap)

	Cpu_thread_component *cpu_thread = nullptr;
	Stored_cpu_thread_info *stored_cpu_thread = stored_cpu_threads.first();
	while(stored_cpu_thread)
	{
		if(stored_cpu_thread->bootstrapped)
		{
			// Identify
			cpu_thread = cpu_session.parent_state().cpu_threads.first();
			if(cpu_thread) cpu_thread = cpu_thread->find_by_name(stored_cpu_thread->name.string());
			if(!cpu_thread)
			{
				Genode::error("Could not find bootstrapped CPU thread for name=", stored_cpu_thread->name);
				throw Genode::Exception();
			}
		}
		else
		{
			// Recreate
			// First, find translation of PD session badge used for creating the CPU thread
			Ckpt_resto_badge_info *cr_info = _ckpt_to_resto_infos.first();
			if(cr_info) cr_info = cr_info->find_by_ckpt_badge(stored_cpu_thread->pd_session_badge);
			if(!cr_info)
			{
				Genode::error("Could not find translation for stored PD session badge=", stored_cpu_thread->pd_session_badge);
				throw Genode::Exception();
			}
			// Find PD session corresponding to the found PD session badge
			Pd_session_component *pd_session = pd_sessions.first();
			if(pd_session) pd_session = pd_session->find_by_badge(cr_info->resto_cap.local_name());
			if(!pd_session)
			{
				Genode::error("Could not find PD session for ", cr_info->resto_cap);
				throw Genode::Exception();
			}
			// Create CPU thread
			Genode::Cpu_thread_capability cap =
					cpu_session.create_thread(pd_session->cap(), stored_cpu_thread->name, stored_cpu_thread->affinity,
							stored_cpu_thread->weight, stored_cpu_thread->utcb);
			cpu_thread = cpu_session.parent_state().cpu_threads.first();
			if(cpu_thread) cpu_thread = cpu_thread->find_by_badge(cap.local_name());
			if(!cpu_thread)
			{
				Genode::error("Could not find newly created CPU thread for ", cap);
				throw Genode::Exception();
			}
			// Store kcap for the badge of the newly created RPC object
			_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_cpu_thread->kcap, cpu_thread->cap()));
		}

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_cpu_thread->badge, cpu_thread->cap()));

		stored_cpu_thread = stored_cpu_thread->next();
	}
}


void Restorer::_identify_recreate_rm_sessions(Rm_root &rm_root, Genode::List<Stored_rm_session_info> &stored_rm_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only no RM session created by now (created by bootstrap)
	if(rm_root.session_infos().first()) Genode::warning("There are already created RM sessions");

	Rm_session_component *rm_session = nullptr;
	Stored_rm_session_info *stored_rm_session = stored_rm_sessions.first();
	while(stored_rm_session)
	{
		// Nothing to identify

		// Recreate
		Genode::Rpc_in_buffer<160> creation_args(stored_rm_session->creation_args.string());
		Genode::Session_capability rm_session_cap = rm_root.session(creation_args, Genode::Affinity());
		rm_session = _child.custom_services().rm_root->session_infos().first();
		if(rm_session) rm_session = rm_session->find_by_badge(rm_session_cap.local_name());
		if(!rm_session)
		{
			Genode::error("Could not find newly created RM session for ", rm_session_cap);
			throw Genode::Exception();
		}
		// Store kcap for the badge of the newly created RPC object
		_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_rm_session->kcap, rm_session->cap()));

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_rm_session->badge, rm_session->cap()));

		_identify_recreate_region_maps(*rm_session, stored_rm_session->stored_region_map_infos);

		stored_rm_session = stored_rm_session->next();
	}
}


void Restorer::_identify_recreate_region_maps(
		Rm_session_component &rm_session, Genode::List<Stored_region_map_info> &stored_region_maps)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be no region maps created
	if(rm_session.parent_state().region_maps.first()) Genode::warning("There are already region maps created, expected none");

	Region_map_component *region_map = nullptr;
	Stored_region_map_info *stored_region_map = stored_region_maps.first();
	while(stored_region_map)
	{
		// Nothing to identify

		// Recreate
		Genode::Capability<Genode::Region_map> cap = rm_session.create(stored_region_map->size);
		region_map = rm_session.parent_state().region_maps.first();
		if(region_map) region_map = region_map->find_by_badge(cap.local_name());
		if(!region_map)
		{
			Genode::error("Could not find newly created region map for ", cap);
			throw Genode::Exception();
		}
		// Store kcap for the badge of the newly created RPC object
		_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_region_map->kcap, region_map->cap()));

		// Insert region map badge/cap
		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_region_map->badge, region_map->cap()));
		// Insert region map's dataspace badge/cap for _restore_state_attached_regions
		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_region_map->ds_badge, region_map->parent_state().ds_cap));


		stored_region_map = stored_region_map->next();
	}
}


void Restorer::_identify_recreate_log_sessions(Log_root &log_root, Genode::List<Stored_log_session_info> &stored_log_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only no LOG session created by now (created by bootstrap)
	if(log_root.session_infos().first()) Genode::warning("There are already created LOG sessions");

	Log_session_component *log_session = nullptr;
	Stored_log_session_info *stored_log_session = stored_log_sessions.first();
	while(stored_log_session)
	{
		// Nothing to identify

		// Recreate
		Genode::Rpc_in_buffer<160> creation_args(stored_log_session->creation_args.string());
		Genode::Session_capability log_session_cap = log_root.session(creation_args, Genode::Affinity());
		log_session = _child.custom_services().log_root->session_infos().first();
		if(log_session) log_session = log_session->find_by_badge(log_session_cap.local_name());
		if(!log_session)
		{
			Genode::error("Could not find newly created LOG session for ", log_session_cap);
			throw Genode::Exception();
		}
		// Store kcap for the badge of the newly created RPC object
		_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_log_session->kcap, log_session->cap()));

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_log_session->badge, log_session->cap()));

		stored_log_session = stored_log_session->next();
	}
}


void Restorer::_identify_recreate_timer_sessions(Timer_root &timer_root, Genode::List<Stored_timer_session_info> &stored_timer_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	// There shall be only no RM session created by now (created by bootstrap)
	if(timer_root.session_infos().first()) Genode::warning("There are already created Timer sessions");

	Timer_session_component *timer_session = nullptr;
	Stored_timer_session_info *stored_timer_session = stored_timer_sessions.first();
	while(stored_timer_session)
	{
		// Nothing to identify

		// Recreate
		Genode::Rpc_in_buffer<160> creation_args(stored_timer_session->creation_args.string());
		Genode::Session_capability timer_session_cap = timer_root.session(creation_args, Genode::Affinity());
		timer_session = _child.custom_services().timer_root->session_infos().first();
		if(timer_session) timer_session = timer_session->find_by_badge(timer_session_cap.local_name());
		if(!timer_session)
		{
			Genode::error("Could not find newly created Timer session for ", timer_session_cap);
			throw Genode::Exception();
		}
		// Store kcap for the badge of the newly created RPC object
		_capability_map_infos.insert(new (_alloc) Cap_kcap_info(stored_timer_session->kcap, timer_session->cap()));

		_ckpt_to_resto_infos.insert(new (_alloc) Ckpt_resto_badge_info(stored_timer_session->badge, timer_session->cap()));

		stored_timer_session = stored_timer_session->next();
	}
}


void Restorer::_restore_state_pd_sessions(Pd_root &pd_root, Genode::List<Stored_pd_session_info> &stored_pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_pd_session_info *stored_pd_session = stored_pd_sessions.first();
	while(stored_pd_session)
	{
		// Find corresponding stored PD session
		Pd_session_component *pd_session = _find_child_object(stored_pd_session->badge, pd_root.session_infos());
		if(!pd_session)
		{
			Genode::error("Could not find child RAM session for ckpt badge ", stored_pd_session->badge);
			throw Genode::Exception();
		}

		// Restore state
		Genode::size_t ram_quota = Genode::Arg_string::find_arg(stored_pd_session->upgrade_args.string(), "ram_quota").ulong_value(0);
		if(ram_quota != 0) pd_root.upgrade(pd_session->cap(), stored_pd_session->upgrade_args.string());

		// Wrap Region_maps of child's and checkpointer's PD session in lists for reusing _restore_state_region_maps
		// The linked list pointers of the three regions maps are usually not used gloablly
		Genode::List<Stored_region_map_info> temp_stored;
		temp_stored.insert(&stored_pd_session->stored_linker_area);
		temp_stored.insert(&stored_pd_session->stored_stack_area);
		temp_stored.insert(&stored_pd_session->stored_address_space);
		Genode::List<Region_map_component> temp_child;
		temp_child.insert(&pd_session->linker_area_component());
		temp_child.insert(&pd_session->stack_area_component());
		temp_child.insert(&pd_session->address_space_component());
		_restore_state_region_maps(temp_child, temp_stored, pd_root.session_infos());

		stored_pd_session = stored_pd_session->next();
	}
}


void Restorer::_restore_state_ram_sessions(Ram_root &ram_root, Genode::List<Stored_ram_session_info> &stored_ram_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_ram_session_info *stored_ram_session = stored_ram_sessions.first();
	while(stored_ram_session)
	{
		// Find corresponding child RAM session
		Ram_session_component *ram_session = _find_child_object(stored_ram_session->badge, ram_root.session_infos());
		if(!ram_session)
		{
			Genode::error("Could not find child RAM session for ckpt badge ", stored_ram_session->badge);
			throw Genode::Exception();
		}

		// Restore state
		Genode::size_t ram_quota = Genode::Arg_string::find_arg(stored_ram_session->upgrade_args.string(), "ram_quota").ulong_value(0);
		if(ram_quota != 0) ram_root.upgrade(ram_session->cap(), stored_ram_session->upgrade_args.string());

		_restore_state_ram_dataspaces(*ram_session, stored_ram_session->stored_ramds_infos);

		stored_ram_session = stored_ram_session->next();
	}
}


void Restorer::_restore_state_ram_dataspaces(
		Ram_session_component &ram_session, Genode::List<Stored_ram_dataspace_info> &stored_ram_dataspaces)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_ram_dataspace_info *stored_ram_dataspace = stored_ram_dataspaces.first();
	while(stored_ram_dataspace)
	{
		// Find corresponding child RAM dataspace
		Ram_dataspace_info *ram_dataspace = _find_child_object(stored_ram_dataspace->badge, ram_session.parent_state().ram_dataspaces);
		if(!ram_dataspace)
		{
			Genode::error("Could not find child RAM dataspace for ckpt badge ", stored_ram_dataspace->badge);
			throw Genode::Exception();
		}

		// Restore state
		// Postpone memory copy to a latter time
		Orig_copy_resto_info *info = new (_alloc) Orig_copy_resto_info(
				ram_dataspace->cap, stored_ram_dataspace->memory_content, 0, stored_ram_dataspace->size);
		_memory_to_restore.insert(info);

		stored_ram_dataspace = stored_ram_dataspace->next();
	}

}


void Restorer::_restore_state_cpu_sessions(Cpu_root &cpu_root, Genode::List<Stored_cpu_session_info> &stored_cpu_sessions,
		Genode::List<Pd_session_component> &pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_cpu_session_info *stored_cpu_session = stored_cpu_sessions.first();
	while(stored_cpu_session)
	{
		// Find corresponding child CPU session
		Cpu_session_component *cpu_session = _find_child_object(stored_cpu_session->badge, cpu_root.session_infos());
		if(!cpu_session)
		{
			Genode::error("Could not find child RAM session for ckpt badge ", stored_cpu_session->badge);
			throw Genode::Exception();
		}

		// Restore state
		// sigh
		if(stored_cpu_session->sigh_badge != 0)
		{
			// Restore sigh
			// Find sigh's resto cap
			Signal_context_info *context = nullptr;
			Pd_session_component *pd_session = pd_sessions.first();
			while(pd_session)
			{
				context = _find_child_object(stored_cpu_session->sigh_badge, pd_session->parent_state().signal_contexts);
				if(context) break;

				pd_session = pd_session->next();
			}
			if(!context)
			{
				Genode::error("Could not find child signal context for ckpt badge ", stored_cpu_session->sigh_badge);
				throw Genode::Exception();
			}
			cpu_session->exception_sigh(context->cap);
		}
		// Upgrade session if necessary
		Genode::size_t ram_quota = Genode::Arg_string::find_arg(stored_cpu_session->upgrade_args.string(), "ram_quota").ulong_value(0);
		if(ram_quota != 0) cpu_root.upgrade(cpu_session->cap(), stored_cpu_session->upgrade_args.string());

		_restore_state_cpu_threads(*cpu_session, stored_cpu_session->stored_cpu_thread_infos, pd_sessions);

		stored_cpu_session = stored_cpu_session->next();
	}
}


void Restorer::_restore_state_cpu_threads(Cpu_session_component &cpu_session, Genode::List<Stored_cpu_thread_info> &stored_cpu_threads,
		Genode::List<Pd_session_component> &pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_cpu_thread_info *stored_cpu_thread = stored_cpu_threads.first();
	while(stored_cpu_thread)
	{
		// Find corresponding child CPU thread
		Cpu_thread_component *cpu_thread = _find_child_object(stored_cpu_thread->badge, cpu_session.parent_state().cpu_threads);
		if(!cpu_thread)
		{
			Genode::error("Could not find child CPU thread for ckpt badge ", stored_cpu_thread->badge);
			throw Genode::Exception();
		}

		// Restore state
		// sigh
		if(stored_cpu_thread->sigh_badge != 0)
		{
			// Restore sigh
			// Find sigh's resto cap
			Signal_context_info *context = nullptr;
			Pd_session_component *pd_session = pd_sessions.first();
			while(pd_session)
			{
				context = _find_child_object(stored_cpu_thread->sigh_badge, pd_session->parent_state().signal_contexts);
				if(context) break;

				pd_session = pd_session->next();
			}
			if(!context)
			{
				Genode::error("Could not find child signal context for ckpt badge ", stored_cpu_thread->sigh_badge);
				throw Genode::Exception();
			}
			cpu_thread->exception_sigh(context->cap);
		}
		// started
		if(stored_cpu_thread->started && !cpu_thread->parent_state().started)
		{
			//cpu_thread->start(stored_cpu_thread->ts.ip, stored_cpu_thread->ts.sp);
		}
		// paused
		if(stored_cpu_thread->paused && !cpu_thread->parent_state().paused)
		{
			//cpu_thread->pause();
		}
		// single_step
		if(stored_cpu_thread->single_step && !cpu_thread->parent_state().single_step)
		{
			cpu_thread->single_step(true);
		}
		// Thread state
		cpu_thread->state(stored_cpu_thread->ts);


		stored_cpu_thread = stored_cpu_thread->next();
	}
}


void Restorer::_restore_state_rm_sessions(Rm_root &rm_root, Genode::List<Stored_rm_session_info> &stored_rm_sessions,
		Genode::List<Pd_session_component> &pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_rm_session_info *stored_rm_session = stored_rm_sessions.first();
	while(stored_rm_session)
	{
		// Find corresponding child RM session
		Rm_session_component *rm_session = _find_child_object(stored_rm_session->badge, rm_root.session_infos());
		if(!rm_session)
		{
			Genode::error("Could not find child RM session for ckpt badge ", stored_rm_session->badge);
			throw Genode::Exception();
		}

		// Restore state
		Genode::size_t ram_quota = Genode::Arg_string::find_arg(stored_rm_session->upgrade_args.string(), "ram_quota").ulong_value(0);
		if(ram_quota != 0) rm_root.upgrade(rm_session->cap(), stored_rm_session->upgrade_args.string());

		_restore_state_region_maps(rm_session->parent_state().region_maps, stored_rm_session->stored_region_map_infos, pd_sessions);

		stored_rm_session = stored_rm_session->next();
	}
}


void Restorer::_restore_state_region_maps(Genode::List<Region_map_component> &region_maps, Genode::List<Stored_region_map_info> &stored_region_maps,
		Genode::List<Pd_session_component> &pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_region_map_info *stored_region_map = stored_region_maps.first();
	while(stored_region_map)
	{
		// Find corresponding child region map
		Region_map_component *region_map = _find_child_object(stored_region_map->badge, region_maps);
		if(!region_map)
		{
			Genode::error("Could not find child region map for ckpt badge ", stored_region_map->badge);
			throw Genode::Exception();
		}

		// Restore state
		// sigh
		if(stored_region_map->sigh_badge != 0)
		{
			// Restore sigh
			// Find sigh's resto cap
			Signal_context_info *context = nullptr;
			Pd_session_component *pd_session = pd_sessions.first();
			while(pd_session)
			{
				context = _find_child_object(stored_region_map->sigh_badge, pd_session->parent_state().signal_contexts);
				if(context) break;

				pd_session = pd_session->next();
			}
			if(!context)
			{
				Genode::error("Could not find child signal context for ckpt badge ", stored_region_map->sigh_badge);
				throw Genode::Exception();
			}
			region_map->fault_handler(context->cap);
		}
		// Attached regions
		_restore_state_attached_regions(*region_map, stored_region_map->stored_attached_region_infos);

		stored_region_map = stored_region_map->next();
	}
}


void Restorer::_restore_state_attached_regions(Region_map_component &region_map,
		Genode::List<Stored_attached_region_info> &stored_attached_regions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_attached_region_info *stored_attached_region = stored_attached_regions.first();
	while(stored_attached_region)
	{
		Attached_region_info *attached_region = nullptr;

		// Find corresponding attached region by the position in the region map or recreate it
		if(stored_attached_region->bootstrapped)
		{
			// Identify
			attached_region = region_map.parent_state().attached_regions.first();
			if(attached_region) attached_region = attached_region->find_by_addr(stored_attached_region->rel_addr);
			if(!attached_region)
			{
				Genode::error("Could not find bootstrapped attached region for attached region ",
						stored_attached_region->badge, " at address ", stored_attached_region->rel_addr);
				throw Genode::Exception();
			}
		}
		else
		{
			// Restore the mapping
			// Find dataspace: _ckpt_to_resto_infos shall have all dataspaces from known services
			// (e.g. dataspaces from RAM sessions, region map's dataspaces),
			// still there could be attached dataspaces whose origin is unknown and, thus, shall be allocated here
			Genode::Dataspace_capability ds_cap;
			Ckpt_resto_badge_info *cr_info = _ckpt_to_resto_infos.first();
			if(cr_info) cr_info = cr_info->find_by_ckpt_badge(stored_attached_region->attached_ds_badge);
			if(cr_info)
			{
				ds_cap = Genode::reinterpret_cap_cast<Genode::Dataspace>(cr_info->resto_cap);
			}
			else
			{
				ds_cap = _child._env.ram().alloc(stored_attached_region->size);

				// Store kcap for the badge of the newly created RPC object
				_capability_map_infos.insert(new (_alloc) Cap_kcap_info(
						stored_attached_region->kcap, attached_region->attached_ds_cap));
			}

			region_map.attach(ds_cap, stored_attached_region->size, stored_attached_region->offset,
					true, stored_attached_region->rel_addr, stored_attached_region->executable);

			attached_region = region_map.parent_state().attached_regions.first();
			if(attached_region) attached_region = attached_region->find_by_addr(stored_attached_region->rel_addr);
			if(!attached_region)
			{
				Genode::error("Could not find recreated attached region for attached region ",
						stored_attached_region->badge, " at address ", stored_attached_region->rel_addr);
				throw Genode::Exception();
			}
		}

		// Find out whether the attached region's memory needs to be restored
		// Find out whether the attached region is a known region map (do not remember region maps)
		Ref_badge *badge = _region_map_dataspaces_from_stored.first();
		if(badge) badge = badge->find_by_badge(stored_attached_region->attached_ds_badge);
		if(!badge)
		{
			// Find out whether the cap is already in memory to restore (only add new dataspaces)
			Orig_copy_resto_info *info = _memory_to_restore.first();
			if(info) info = info->find_by_copy_badge(stored_attached_region->memory_content.local_name());
			if(!info)
			{
				// If not in list, then insert it
				info = new (_alloc) Orig_copy_resto_info(
						attached_region->attached_ds_cap, stored_attached_region->memory_content, 0, stored_attached_region->size);
				_memory_to_restore.insert(info);
			}
		}

		stored_attached_region = stored_attached_region->next();
	}
}


void Restorer::_restore_state_log_sessions(Log_root &log_root, Genode::List<Stored_log_session_info> &stored_log_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_log_session_info *stored_log_session = stored_log_sessions.first();
	while(stored_log_session)
	{
		// Find corresponding child LOG session
		Log_session_component *log_session = _find_child_object(stored_log_session->badge, log_root.session_infos());

		// Restore state
		Genode::size_t ram_quota = Genode::Arg_string::find_arg(stored_log_session->upgrade_args.string(), "ram_quota").ulong_value(0);
		if(ram_quota != 0) log_root.upgrade(log_session->cap(), stored_log_session->upgrade_args.string());

		stored_log_session = stored_log_session->next();
	}
}


void Restorer::_restore_state_timer_sessions(Timer_root &timer_root, Genode::List<Stored_timer_session_info> &stored_timer_sessions,
		Genode::List<Pd_session_component> &pd_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Stored_timer_session_info *stored_timer_session = stored_timer_sessions.first();
	while(stored_timer_session)
	{
		// Find corresponding child Timer session
		Timer_session_component *timer_session = _find_child_object(stored_timer_session->badge, timer_root.session_infos());

		// Restore state
		// sigh
		if(stored_timer_session->sigh_badge != 0)
		{
			// Find Signal context info
			Signal_context_info *context = nullptr;
			Pd_session_component *pd_session = pd_sessions.first();
			while(pd_session)
			{
				context = _find_child_object(stored_timer_session->sigh_badge, pd_session->parent_state().signal_contexts);
				if(context) break;

				pd_session = pd_session->next();
			}
			if(!context)
			{
				Genode::error("Could not find child signal context for ckpt badge ", stored_timer_session->sigh_badge);
				throw Genode::Exception();
			}
			timer_session->sigh(context->cap);
		}
		// timeout and periodic
		if(stored_timer_session->timeout != 0 && stored_timer_session->periodic)
		{
			timer_session->trigger_periodic(stored_timer_session->timeout);
		}
		else if(stored_timer_session->timeout != 0 && !stored_timer_session->periodic)
		{
			timer_session->trigger_once(stored_timer_session->timeout);
		}
		// Upgrade session if necessary
		Genode::size_t ram_quota = Genode::Arg_string::find_arg(stored_timer_session->upgrade_args.string(), "ram_quota").ulong_value(0);
		if(ram_quota != 0) timer_root.upgrade(timer_session->cap(), stored_timer_session->upgrade_args.string());

		stored_timer_session = stored_timer_session->next();
	}
}


void Restorer::_resolve_inc_checkpoint_dataspaces(
		Genode::List<Ram_session_component> &ram_sessions, Genode::List<Orig_copy_resto_info> &memory_infos)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Ram_session_component *ram_session = ram_sessions.first();
	while(ram_session)
	{
		Ram_dataspace_info *ramds_info = ram_session->parent_state().ram_dataspaces.first();
		while(ramds_info)
		{
			// RAM dataspace is managed for inc ckpt
			if(ramds_info->mrm_info)
			{
				// Find corresponding memory_info
				Orig_copy_resto_info *memory_info = memory_infos.first();
				if(memory_info) memory_info = memory_info->find_by_orig_badge(ramds_info->cap.local_name());
				if(memory_info)
				{
					// Now we found a memory_info which is actually managed by the inc ckpt mechanism
					// Thus, replace this memory_info with the attached designated dataspaces
					// and clean up the old memory_info
					memory_infos.remove(memory_info);

					Designated_dataspace_info *dd_info = ramds_info->mrm_info->dd_infos.first();
					if(dd_info && dd_info->attached)
					{
						Orig_copy_resto_info *new_oc_info = new (_alloc) Orig_copy_resto_info(dd_info->cap,
								memory_info->copy_ds_cap, dd_info->rel_addr, dd_info->size);
						memory_infos.insert(new_oc_info);

						dd_info = dd_info->next();
					}

					Genode::destroy(_alloc, memory_info);
				}

			}

			ramds_info = ramds_info->next();
		}

		ram_session = ram_session->next();
	}

}


void Restorer::_restore_cap_map(Target_child &child, Target_state &state)
{
	using Genode::size_t;
	using Genode::addr_t;
	using Genode::log;
	using Genode::Hex;

	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	addr_t child_cap_idx_alloc_addr = Genode::Foc_native_pd_client(child.pd().native_pd()).cap_map_info();
	addr_t state_cap_idx_alloc_addr = state._cap_idx_alloc_addr;
	//log("Cap_idx_alloc: child addr=", Hex(child_cap_idx_alloc_addr), ", state addr=", Hex(state_cap_idx_alloc_addr));

	// Find attached region containing child's cap_idx_alloc struct
	Attached_region_info *attached_region = nullptr;
	{
		attached_region = child.pd().address_space_component().parent_state().attached_regions.first();
		if(attached_region) attached_region = attached_region->find_by_addr(child_cap_idx_alloc_addr);
		if(!attached_region)
		{
			Genode::error("Could not find child's dataspace containing the cap_idx_alloc struct with the address ", Hex(child_cap_idx_alloc_addr));
			throw Genode::Exception();
		}
	}
	//Find stored attached region containing state's cap_idx_alloc struct
	Stored_attached_region_info *stored_attached_region = nullptr;
	{
		Stored_pd_session_info *stored_pd_session = state._stored_pd_sessions.first();
		if(stored_pd_session) stored_pd_session = stored_pd_session->find_by_bootstrapped(true);
		if(!stored_pd_session)
		{
			Genode::error("Could not find bootstrapped stored PD session");
			throw Genode::Exception();
		}
		stored_attached_region = stored_pd_session->stored_address_space.stored_attached_region_infos.first();
		if(stored_attached_region) stored_attached_region = stored_attached_region->find_by_addr(state_cap_idx_alloc_addr);
		if(!stored_attached_region)
		{
			Genode::error("Could not find stored dataspace containing the cap_idx_alloc struct with the address ", Hex(state_cap_idx_alloc_addr));
			throw Genode::Exception();
		}
	}

	size_t const struct_size    = sizeof(Genode::Cap_index_allocator_tpl<Genode::Cap_index,4096>);
	size_t const array_ele_size = sizeof(Genode::Cap_index);
	size_t const array_size     = array_ele_size*4096;

	/**
	 * Array element layout as a list element (Changed from Avl tree)
	 *
	 *             32 bit                8 bit   8 bit       16 bit
	 * +-------------------------------+-------+-------+---------------+
	 * |        list pointer           | r_cnt |  res  |     badge     |
	 * +-------------------------------+-------+-------+---------------+
	 *
	 * list pointer has a pointers valid in the address space of the child
	 * r_cnt        is a number counting the references of the Cap_index
	 * res          is padding
	 * badge        is the system global identifier for Cap_indices
	 */

	addr_t const remote_child_ds_start = attached_region->rel_addr;
	addr_t const remote_child_ds_end   = remote_child_ds_start + attached_region->size;
	addr_t const remote_child_struct_start = child_cap_idx_alloc_addr;
	addr_t const remote_child_struct_end   = remote_child_struct_start + struct_size;
	addr_t const remote_child_array_start = remote_child_struct_start + 8;
	addr_t const remote_child_array_end   = remote_child_array_start + array_size;

	addr_t const local_child_ds_start = state._env.rm().attach(attached_region->attached_ds_cap);
	addr_t const local_child_ds_end   = local_child_ds_start + attached_region->size;
	addr_t const local_child_struct_start = local_child_ds_start + (remote_child_struct_start - remote_child_ds_start);
	addr_t const local_child_struct_end   = local_child_struct_start + struct_size;
	addr_t const local_child_array_start = local_child_struct_start + 8;
	addr_t const local_child_array_end   = local_child_array_start + array_size;

	addr_t const local_state_ds_start = state._env.rm().attach(stored_attached_region->memory_content);
	addr_t const local_state_ds_end   = local_state_ds_start + stored_attached_region->size;
	addr_t const local_state_struct_start = local_state_ds_start + (state_cap_idx_alloc_addr - remote_child_ds_start);
	addr_t const local_state_struct_end   = local_state_struct_start + struct_size;
	addr_t const local_state_array_start = local_state_struct_start + 8;
	addr_t const local_state_array_end   = local_state_array_start + array_size;

	if(verbose_debug)
	{
		log("remote_child_ds_start:     ", Hex(remote_child_ds_start));
		log("remote_child_struct_start: ", Hex(remote_child_struct_start));
		log("remote_child_array_start:  ", Hex(remote_child_array_start));
		log("remote_child_array_end:    ", Hex(remote_child_array_end));
		log("remote_child_struct_end:   ", Hex(remote_child_struct_end));
		log("remote_child_ds_end:       ", Hex(remote_child_ds_end));

		log("local_child_ds_start:     ", Hex(local_child_ds_start));
		log("local_child_struct_start: ", Hex(local_child_struct_start));
		log("local_child_array_start:  ", Hex(local_child_array_start));
		log("local_child_array_end:    ", Hex(local_child_array_end));
		log("local_child_struct_end:   ", Hex(local_child_struct_end));
		log("local_child_ds_end:       ", Hex(local_child_ds_end));

		log("local_state_ds_start:     ", Hex(local_state_ds_start));
		log("local_state_struct_start: ", Hex(local_state_struct_start));
		log("local_state_array_start:  ", Hex(local_state_array_start));
		log("local_state_array_end:    ", Hex(local_state_array_end));
		log("local_state_struct_end:   ", Hex(local_state_struct_end));
		log("local_state_ds_end:       ", Hex(local_state_ds_end));
	}

	// copy array from child to state
	{
		Genode::memcpy((void*)local_state_array_start, (void*)local_child_array_start, array_size);
		//dump_mem((void*)(local_state_array_start + 0x200*array_ele_size), 0x100);
	}

	// Replace badges and list pointers in state
	{
		// Find Cap_index with valid list pointer where the new Cap_indices will be attached to
		addr_t previous_cap_index = 0;
		for(unsigned cap_index = 0x200; cap_index < 0x280; ++cap_index)
		{
			addr_t const current_pos = local_state_array_start + cap_index*array_ele_size;
			//log("offset=", Hex(offset), ": ", Hex(*(Genode::uint32_t*) current_pos));
			if(*(Genode::uint32_t*) current_pos)
			{
				previous_cap_index = cap_index;
				break;
			}
		}
		//log("previous: ", Hex(previous_cap_index));

		Cap_kcap_info *cap_info = _capability_map_infos.first();
		while(cap_info)
		{
			if(cap_info->kcap == 0)
			{
				Genode::warning("kcap = 0 for cap ", cap_info->cap);
			}
			else
			{
				addr_t const previous_pos = local_state_array_start + previous_cap_index*array_ele_size;
				addr_t const current_cap_index = cap_info->kcap >> 12;
				addr_t const current_pos = local_state_array_start + current_cap_index*array_ele_size;

				if(!(current_pos >= local_state_array_start && current_pos < local_state_array_end) ||
						!(previous_pos >= local_state_array_start && previous_pos < local_state_array_end))
				{
					Genode::error("Cap_index positions are invalid! Allowed region: [",
							Hex(local_state_array_start), ", ", Hex(local_state_array_end), ") ",
							"current_pos=", Hex(current_pos), ", previous_pos=", Hex(previous_pos));
					throw Genode::Exception();
				}

				//log("previous cap index = ", Hex(previous_cap_index), ", current cap index = ", Hex(current_cap_index));

				// Current state: previous -> next
				//log("Before:");
				//dump_mem((void*)current_pos, 8);
				//dump_mem((void*)previous_pos, 8);

				if(*(Genode::uint16_t*) (current_pos+6))
				{
					Genode::warning("Overriding existing badge at cap_index=", Hex(current_cap_index));
				}

				// Insert List pointer, thus, current points to next
				*(Genode::uint32_t*) current_pos = *(Genode::uint32_t*)previous_pos;
				// Insert ref_count
				*(Genode::uint8_t*) (current_pos+4) = 2;
				// Insert badge
				*(Genode::uint16_t*) (current_pos+6) = cap_info->cap.local_name();

				// Insert List pointer, thus, previous points to current
				*(Genode::uint32_t*) previous_pos = remote_child_array_start + current_cap_index*array_ele_size;

				// State now: previous -> current -> next
				//log("\nAfter:");
				//dump_mem((void*)current_pos, 8);
				//dump_mem((void*)previous_pos, 8);
				//log("\n\n");

				previous_cap_index = current_cap_index;
			}

			cap_info = cap_info->next();
		}
	}

	state._env.rm().detach(local_state_ds_start);
	state._env.rm().detach(local_child_ds_start);

}


void Restorer::_restore_cap_space(Target_child &child)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Cap_kcap_info *ck_info = _capability_map_infos.first();
	while(ck_info)
	{
		if(ck_info->kcap == 0)
		{
			Genode::warning("kcap = 0 for cap ", ck_info->cap);
		}
		else
		{
			// Install capabilities to the child's cap space
			Genode::Foc_native_pd_client(child.pd().native_pd()).install(ck_info->cap, ck_info->kcap);
		}

		ck_info = ck_info->next();
	}
}


void Restorer::_restore_dataspaces(Genode::List<Orig_copy_resto_info> &memory_infos)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	Orig_copy_resto_info *memory_info = memory_infos.first();
	while(memory_info)
	{
		if(!memory_info->restored)
		{
			_restore_dataspace_content(memory_info->orig_ds_cap, memory_info->copy_ds_cap,
					memory_info->copy_rel_addr, memory_info->copy_size);
			memory_info->restored = true;
		}

		memory_info = memory_info->next();
	}
}


void Restorer::_restore_dataspace_content(Genode::Dataspace_capability orig_ds_cap,
		Genode::Ram_dataspace_capability copy_ds_cap, Genode::addr_t copy_rel_addr, Genode::size_t copy_size)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(orig ", orig_ds_cap,
			", copy ", copy_ds_cap, ", copy_rel_addr=", Genode::Hex(copy_rel_addr),
			", copy_size=", Genode::Hex(copy_size), ")");

	char *orig = _state._env.rm().attach(orig_ds_cap);
	char *copy = _state._env.rm().attach(copy_ds_cap);

	Genode::memcpy(copy + copy_rel_addr, orig, copy_size);

	_state._env.rm().detach(copy);
	_state._env.rm().detach(orig);
}


Restorer::Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state)
: _alloc(alloc), _child(child), _state(state) { }


Restorer::~Restorer()
{
	_destroy_list(_capability_map_infos);
	_destroy_list(_ckpt_to_resto_infos);
	_destroy_list(_memory_to_restore);
	_destroy_list(_region_map_dataspaces_from_stored);
}


void Restorer::restore()
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m()");

	Genode::log("Before: \n", _child);

	// Create a list of known region map's dataspace capabilities
	// It is used to identify region maps which are attached to region maps
	// when bookmarking dataspace content for restoration
	_region_map_dataspaces_from_stored = _create_region_map_dataspaces(_state._stored_pd_sessions, _state._stored_rm_sessions);

	if(verbose_debug)
	{
		Genode::log("Region map dataspaces (from ckpt):");
		Ref_badge *info = _region_map_dataspaces_from_stored.first();
		if(!info) Genode::log(" <empty>\n");
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Identify or create RPC objects (caution: PD <- CPU thread <- Native cap ( "<-" means requires))
	//   Make a translation of old badges to new badges
	//   In the restore state phase it is used when iterating through the stored states of RPC objects
	//   and finding the corresponding child object
	_identify_recreate_ram_sessions(*_child.custom_services().ram_root, _state._stored_ram_sessions);
	_identify_recreate_pd_sessions(*_child.custom_services().pd_root, _state._stored_pd_sessions);
	_identify_recreate_cpu_sessions(*_child.custom_services().cpu_root, _state._stored_cpu_sessions,
			_child.custom_services().pd_root->session_infos());
	if(_state._stored_rm_sessions.first())
	{
		_child.custom_services().find("RM"); // Implicitly creates root object
		_identify_recreate_rm_sessions(*_child.custom_services().rm_root, _state._stored_rm_sessions);
	}
	if(_state._stored_log_sessions.first())
	{
		_child.custom_services().find("LOG"); // Implicitly creates root object
		_identify_recreate_log_sessions(*_child.custom_services().log_root, _state._stored_log_sessions);
	}
	if(_state._stored_timer_sessions.first())
	{
		_child.custom_services().find("Timer"); // Implicitly creates root object
		_identify_recreate_timer_sessions(*_child.custom_services().timer_root, _state._stored_timer_sessions);
	}


	if(verbose_debug)
	{
		Genode::log("Ckpt to resto infos:");
		Ckpt_resto_badge_info *info = _ckpt_to_resto_infos.first();
		if(!info) Genode::log(" <empty>\n");
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Restore state of all objects using the translation of old badges to new badges
	// (caution: RAM <- Region map (incl. PD session) ( "<-" means requires))
	//   Also make a mapping of memory to restore
	_restore_state_ram_sessions(*_child.custom_services().ram_root, _state._stored_ram_sessions);
	_restore_state_pd_sessions(*_child.custom_services().pd_root, _state._stored_pd_sessions);
	_restore_state_cpu_sessions(*_child.custom_services().cpu_root, _state._stored_cpu_sessions,
			_child.custom_services().pd_root->session_infos());
	if(_child.custom_services().rm_root)
	{
		_restore_state_rm_sessions(*_child.custom_services().rm_root, _state._stored_rm_sessions,
				_child.custom_services().pd_root->session_infos());
	}
	if(_child.custom_services().log_root)
	{
		_restore_state_log_sessions(*_child.custom_services().log_root, _state._stored_log_sessions);
	}
	if(_child.custom_services().timer_root)
	{
		_restore_state_timer_sessions(*_child.custom_services().timer_root, _state._stored_timer_sessions,
				_child.custom_services().pd_root->session_infos());
	}


	if(verbose_debug)
	{
		Genode::log("Capability map infos:");
		Cap_kcap_info const *info = _capability_map_infos.first();
		if(!info) Genode::log(" <empty>\n");
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}


	// Resolve inc checkpoint dataspaces in memory to restore
	_resolve_inc_checkpoint_dataspaces(_child.custom_services().ram_root->session_infos(), _memory_to_restore);

	// Replace old badges with new in capability map
	_restore_cap_map(_child, _state);

	if(verbose_debug)
	{
		Genode::log("Memory to restore:");
		Orig_copy_resto_info *info = _memory_to_restore.first();
		if(!info) Genode::log(" <empty>\n");
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Insert capabilities of all objects into capability space
	_restore_cap_space(_child);

	// Copy stored content to child content
	_restore_dataspaces(_memory_to_restore);

	// Clean up
	_destroy_list(_capability_map_infos);
	_destroy_list(_ckpt_to_resto_infos);
	_destroy_list(_memory_to_restore);
	_destroy_list(_region_map_dataspaces_from_stored);

	Genode::log("After: \n", _child);

}
