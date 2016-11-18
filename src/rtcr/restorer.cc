/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "restorer.h"

using namespace Rtcr;


void Restorer::_restore_pd_session(Pd_session_component &child_obj, Stored_pd_session_info &state_info)
{
	_restore_sources(child_obj, state_info.stored_source_infos);
	_restore_contexts(child_obj, state_info.stored_context_infos);
	_restore_region_map(child_obj.address_space_component(), state_info.stored_address_space);
	_restore_region_map(child_obj.stack_area_component(), state_info.stored_stack_area);
	_restore_region_map(child_obj.linker_area_component(), state_info.stored_linker_area);
}


void Restorer::_restore_sources(Pd_session_component &child_obj, Genode::List<Stored_signal_source_info> &state_infos)
{
	Stored_signal_source_info *state_info = nullptr;
	while(state_info)
	{
		// Create RPC object
		Genode::Capability<Genode::Signal_source> source_cap = child_obj.alloc_signal_source();

		// Create translation of ckpt_badge to resto_badge
		Badge_badge_info *info = new (_alloc) Badge_badge_info(state_info->badge, source_cap.local_name());
		_badge_dictionary.insert(info);

		state_info = state_info->next();
	}
}


void Restorer::_restore_contexts(Pd_session_component &child_obj, Genode::List<Stored_signal_context_info> &state_infos)
{
	Stored_signal_context_info *state_info = nullptr;
	while(state_info)
	{
		// Find source_cap in PD session
		Genode::Capability<Genode::Signal_source> source_cap;
		if(state_info->signal_source_badge != 0)
		{
			// Find badge of restored RPC object
			Badge_badge_info *badge_info = _badge_dictionary.first();
			if(badge_info) badge_info = badge_info->find_by_ckpt_badge(state_info->badge);
			if(!badge_info)
			{
				Genode::error("No resto badge for ckpt badge ", state_info->badge," of signal source found!");
				throw Genode::Exception();
			}

			// Find restored RPC object
			Signal_source_info *ss_info = child_obj.signal_source_infos().first();
			if(ss_info) ss_info = ss_info->find_by_badge(badge_info->resto_badge);
			if(!ss_info)
			{
				Genode::error("No signal source for resto badge ", badge_info->resto_badge, " found!");
				throw Genode::Exception();
			}

			source_cap = ss_info->cap;
		}

		// Create RPC object
		Genode::Capability<Genode::Signal_context> context_cap = child_obj.alloc_context(source_cap, state_info->imprint);

		// Create translation of ckpt_badge to resto_badge
		Badge_badge_info *info = new (_alloc) Badge_badge_info(state_info->badge, context_cap.local_name());
		_badge_dictionary.insert(info);

		state_info = state_info->next();
	}
}


void Restorer::_restore_region_map(Region_map_component &child_obj, Stored_region_map_info &state_info)
{

}


void Restorer::_destroy_badge_dictionary()
{
	while(Badge_badge_info *info = _badge_dictionary.first())
	{
		_badge_dictionary.remove(info);
		Genode::destroy(_alloc, info);
	}
}


Restorer::Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state)
: _alloc(alloc), _child(child), _state(state), _badge_dictionary() { }


Restorer::~Restorer()
{
	_destroy_badge_dictionary();
}


void Restorer::restore()
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m()");
	//Create RPC objects with checkpointed state
	//_prepare_rm_sessions();
	//_prepare_log_sessions();
	//_prepare_timer_sessions();

	//Recreate state of automatically created RPC objects
	//PD
	_restore_pd_session(_child.pd(), _state._stored_pd_session);
	//CPU
	//_prepare_cpu_session();
	//  _prepare_threads();
	//RAM
	//_prepare_ram_session();
	//  _prepare_allocated_dataspaces();


	//Adjust capability map/capability array
	//_prepare_capability_map();

	//Insert capabilities into capability space
	//_prepare_capability_space();

	//Genode::List<Badge_info> exclude_infos = _create_exclude_infos(address_space, stack_area, linker_area, rm_root);

	//Copy memory content
	//Genode::List<Badge_dataspace_info> visited_infos;
	//_update_dataspace_content(ram, visited_infos, exclude_infos);
	//_update_dataspace_content(address_space, visited_infos, exclude_infos);
	//_update_dataspace_content(stack_area, visited_infos, exclude_infos);
	//_update_dataspace_content(linker_area, visited_infos, exclude_infos);
	//if(rm_root) _update_dataspace_content(rm_root, visisted_infos, exclude_infos);


	//Delete exclude_infos
	//_destroy_exclude_infos(exclude_infos);
	//Delete visited_infos
	//while(Badge_dataspace_info *info = visited_infos.first())
	//{
	//	visited_infos.remove(info);
	//	Genode::destroy(_alloc, info);
	//}
	_destroy_badge_dictionary();

}
