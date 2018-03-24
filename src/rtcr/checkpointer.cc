/*
 * \brief  Checkpointer of Target_state
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "checkpointer.h"
//#include "util/debug.h"
#include <base/internal/cap_map.h>
#include <base/internal/cap_alloc.h>

using namespace Rtcr;

template<typename T>
void Checkpointer::_destroy_list(Genode::List<T> &list)
{
	while(T *elem = list.first())
	{
		list.remove(elem);
		Genode::destroy(_alloc, elem);
	}
}
template void Checkpointer::_destroy_list(Genode::List<Kcap_badge_info> &list);
template void Checkpointer::_destroy_list(Genode::List<Dataspace_translation_info> &list);
template void Checkpointer::_destroy_list(Genode::List<Ref_badge_info> &list);

void Checkpointer::_destroy_list(Genode::List<Simplified_managed_dataspace_info> &list)
{
	while(Simplified_managed_dataspace_info *smd_info = list.first())
	{
		list.remove(smd_info);

		while(Simplified_managed_dataspace_info::Simplified_designated_ds_info *sdd_info =
				smd_info->designated_dataspaces.first())
		{
			smd_info->designated_dataspaces.remove(sdd_info);
			Genode::destroy(_alloc, sdd_info);
		}

		Genode::destroy(_alloc, smd_info);
	}
}


Genode::List<Kcap_badge_info> Checkpointer::_create_kcap_mappings()
{
	using Genode::log;
	using Genode::Hex;
	using Genode::addr_t;
	using Genode::size_t;
	using Genode::uint16_t;
	const bool verbose_kcap_mappings_debug = false;

	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");

	Genode::List<Kcap_badge_info> result;

	// Retrieve cap_idx_alloc_addr
	Genode::Pd_session_client pd_client(_child.pd().parent_cap());
	addr_t const cap_idx_alloc_addr = Genode::Foc_native_pd_client(pd_client.native_pd()).cap_map_info();
	_state._cap_idx_alloc_addr = cap_idx_alloc_addr;

	if(verbose_kcap_mappings_debug) Genode::log("Address of cap_idx_alloc = ", Hex(cap_idx_alloc_addr));

	// Find child's dataspace containing the capability map
	// It is found via cap_idx_alloc_addr
	Attached_region_info *ar_info = _child.pd().address_space_component().parent_state().attached_regions.first();
	if(ar_info) ar_info = ar_info->find_by_addr(cap_idx_alloc_addr);
	if(!ar_info)
	{
		Genode::error("No dataspace found for cap_idx_alloc's datastructure at ", Hex(cap_idx_alloc_addr));
		throw Genode::Exception();
	}

	// If ar_info is a managed Ram_dataspace_info, mark detached Designated_dataspace_infos and attach them,
	// thus, the Checkpointer does not trigger page faults which mark accessed regions
	Genode::List<Ref_badge_info> marked_badge_infos =
			_mark_and_attach_designated_dataspaces(*ar_info);

	// Create new badge_kcap list
	size_t const struct_size    = sizeof(Genode::Cap_index_allocator_tpl<Genode::Cap_index,4096>);
	size_t const array_ele_size = sizeof(Genode::Cap_index);
	size_t const array_size     = array_ele_size*4096;

	addr_t const child_ds_start     = ar_info->rel_addr;
	addr_t const child_ds_end       = child_ds_start + ar_info->size;
	addr_t const child_struct_start = cap_idx_alloc_addr;
	addr_t const child_struct_end   = child_struct_start + struct_size;
	addr_t const child_array_start  = child_struct_start + 8;
	addr_t const child_array_end    = child_array_start + array_size;

	addr_t const local_ds_start     = _state._env.rm().attach(ar_info->attached_ds_cap, ar_info->size, ar_info->offset);
	addr_t const local_ds_end       = local_ds_start + ar_info->size;
	addr_t const local_struct_start = local_ds_start + (cap_idx_alloc_addr - child_ds_start);
	addr_t const local_struct_end   = local_struct_start + struct_size;
	addr_t const local_array_start  = local_struct_start + 8;
	addr_t const local_array_end    = local_array_start + array_size;

	if(verbose_kcap_mappings_debug)
	{
		log("child_ds_start:     ", Hex(child_ds_start));
		log("child_struct_start: ", Hex(child_struct_start));
		log("child_array_start:  ", Hex(child_array_start));
		log("child_array_end:    ", Hex(child_array_end));
		log("child_struct_end:   ", Hex(child_struct_end));
		log("child_ds_end:       ", Hex(child_ds_end));

		log("local_ds_start:     ", Hex(local_ds_start));
		log("local_struct_start: ", Hex(local_struct_start));
		log("local_array_start:  ", Hex(local_array_start));
		log("local_array_end:    ", Hex(local_array_end));
		log("local_struct_end:   ", Hex(local_struct_end));
		log("local_ds_end:       ", Hex(local_ds_end));
	}

	//dump_mem((void*)local_array_start, 0x1200);

	enum { UNUSED = 0, INVALID_ID = 0xffff };
	for(addr_t curr = local_array_start; curr < local_array_end; curr += array_ele_size)
	{

		size_t const badge_offset = 6;

		// Convert address to pointer and dereference it
		uint16_t const badge = *(uint16_t*)(curr + badge_offset);
		// Current capability map slot = Compute distance from start to current address of array and
		// divide it by the element size;
		// kcap = current capability map slot shifted by 12 bits to the left (last 12 bits are used
		// by Fiasco.OC for parameters for IPC calls)
		addr_t const kcap  = ((curr - local_array_start) / array_ele_size) << 12;

		if(badge != UNUSED && badge != INVALID_ID)
		{
			Kcap_badge_info *state_info = new (_alloc) Kcap_badge_info(kcap, badge);
			result.insert(state_info);

			if(verbose_kcap_mappings_debug) log("+ ", Hex(kcap), ": ", badge, " (", Hex(badge), ")");
		}
	}

	_state._env.rm().detach(local_ds_start);

	// Detach the previously attached Designated_dataspace_infos and delete the list containing marked Designated_dataspace_infos
	_detach_and_unmark_designated_dataspaces(marked_badge_infos, *ar_info);

	return result;
}


Genode::List<Ref_badge_info> Checkpointer::_mark_and_attach_designated_dataspaces(Attached_region_info &ar_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::List<Ref_badge_info> result_infos;

	Managed_region_map_info *mrm_info = ar_info.managed_dataspace(_child.ram().parent_state().ram_dataspaces);
	if(mrm_info)
	{
		Designated_dataspace_info *dd_info = mrm_info->dd_infos.first();
		while(dd_info)
		{
			if(!dd_info->attached)
			{
				dd_info->attach();

				Ref_badge_info *new_info = new (_alloc) Ref_badge_info(dd_info->cap.local_name());
				result_infos.insert(new_info);
			}

			dd_info = dd_info->next();
		}
	}

	return result_infos;
}


void Checkpointer::_detach_and_unmark_designated_dataspaces(Genode::List<Ref_badge_info> &badge_infos, Attached_region_info &ar_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Managed_region_map_info *mrm_info = ar_info.managed_dataspace(_child.ram().parent_state().ram_dataspaces);
	if(mrm_info && badge_infos.first())
	{
		Designated_dataspace_info *dd_info = mrm_info->dd_infos.first();
		while(dd_info)
		{
			if(badge_infos.first()->find_by_badge(dd_info->cap.local_name()))
			{
				dd_info->detach();
			}

			dd_info = dd_info->next();
		}
	}

	// Delete list elements from badge_infos
	_destroy_list(badge_infos);
}


Genode::addr_t Checkpointer::_find_kcap_by_badge(Genode::uint16_t badge)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::addr_t kcap = 0;

	Kcap_badge_info *info = _kcap_mappings.first();
	if(info) info = info->find_by_badge(badge);
	if(info) kcap = info->kcap;

	return kcap;
}


Genode::Dataspace_capability Checkpointer::_find_stored_dataspace(Genode::uint16_t badge)
{
	Genode::Dataspace_capability result;

	// RAM dataspace
	result = _find_stored_dataspace(badge, _state._stored_ram_sessions);
	if(result.valid()) return result;

	// Attached region in PD
	result = _find_stored_dataspace(badge, _state._stored_pd_sessions);
	if(result.valid()) return result;

	// Attached region in RM
	result = _find_stored_dataspace(badge, _state._stored_rm_sessions);
	if(result.valid()) return result;

	return result;
}


Genode::Dataspace_capability Checkpointer::_find_stored_dataspace(Genode::uint16_t badge,
		Genode::List<Stored_ram_session_info> &state_infos)
{
	Genode::Dataspace_capability result;

	Stored_ram_session_info *session_info = state_infos.first();
	while(session_info)
	{
		Stored_ram_dataspace_info *ramds_info = session_info->stored_ramds_infos.first();
		if(ramds_info) ramds_info = ramds_info->find_by_badge(badge);
		if(ramds_info)
		{
			return ramds_info->memory_content;
		}

		session_info = session_info->next();
	}

	return result;
}


Genode::Dataspace_capability Checkpointer::_find_stored_dataspace(Genode::uint16_t badge,
		Genode::List<Stored_pd_session_info> &state_infos)
{
	Genode::Dataspace_capability result;

	Stored_pd_session_info *session_info = state_infos.first();
	while(session_info)
	{
		result = _find_stored_dataspace(badge, session_info->stored_address_space.stored_attached_region_infos);
		if(result.valid()) return result;

		result = _find_stored_dataspace(badge, session_info->stored_stack_area.stored_attached_region_infos);
		if(result.valid()) return result;

		result = _find_stored_dataspace(badge, session_info->stored_linker_area.stored_attached_region_infos);
		if(result.valid()) return result;

		session_info = session_info->next();
	}

	return result;
}


Genode::Dataspace_capability Checkpointer::_find_stored_dataspace(Genode::uint16_t badge,
		Genode::List<Stored_rm_session_info> &state_infos)
{
	Genode::Dataspace_capability result;

	Stored_rm_session_info *session_info = state_infos.first();
	while(session_info)
	{
		Stored_region_map_info *rm_info = session_info->stored_region_map_infos.first();
		while(rm_info)
		{
			result = _find_stored_dataspace(badge, rm_info->stored_attached_region_infos);
			if(result.valid()) return result;

			rm_info = rm_info->next();
		}

		session_info = session_info->next();
	}

	return result;
}


Genode::Dataspace_capability Checkpointer::_find_stored_dataspace(Genode::uint16_t badge,
		Genode::List<Stored_attached_region_info> &state_infos)
{
	Genode::Dataspace_capability result;

	Stored_attached_region_info *info = state_infos.first();
	if(info) info = info->find_by_badge(badge);
	if(info) return info->memory_content;

	return result;
}


void Checkpointer::_prepare_rm_sessions(Genode::List<Stored_rm_session_info> &stored_infos, Genode::List<Rm_session_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Rm_session_component *child_info = nullptr;
	Stored_rm_session_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_state._alloc) Stored_rm_session_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		_prepare_region_maps(stored_info->stored_region_map_infos, child_info->parent_state().region_maps);

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_rm_session_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_rm_session(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_rm_session(Stored_rm_session_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Stored_region_map_info *info = stored_info.stored_region_map_infos.first())
	{
		stored_info.stored_region_map_infos.remove(info);
		_destroy_stored_region_map(*info);
	}
	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_region_maps(Genode::List<Stored_region_map_info> &stored_infos, Genode::List<Region_map_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Region_map_component *child_info = nullptr;
	Stored_region_map_info *stored_info = nullptr;

	// Update stored_info from child_info
	// If a child_info has no corresponding stored_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		Genode::log("Cap: ", child_info->cap(), ", Ds-cap: ", child_info->dataspace());

		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_state._alloc) Stored_region_map_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
		_prepare_attached_regions(stored_info->stored_attached_region_infos, child_info->parent_state().attached_regions);

		// Remeber region map's dataspace badge to remove the dataspace from _memory_to_checkpoint later
		Ref_badge_info *ref_badge = new (_alloc) Ref_badge_info(child_info->parent_state().ds_cap.local_name());
		_region_maps.insert(ref_badge);

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_region_map_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_region_map(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_region_map(Stored_region_map_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Stored_attached_region_info *info = stored_info.stored_attached_region_infos.first())
	{
		stored_info.stored_attached_region_infos.remove(info);
		_destroy_stored_attached_region(*info);
	}
	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_attached_regions(Genode::List<Stored_attached_region_info> &stored_infos, Genode::List<Attached_region_info> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Attached_region_info *child_info = nullptr;
	Stored_attached_region_info *stored_info = nullptr;

	// Update stored_info from child_info
	// If a child_info has no corresponding stored_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_addr(child_info->rel_addr);

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			stored_info = &_create_stored_attached_region(*child_info);
			stored_infos.insert(stored_info);
		}

		// No need to update stored_info

		// Remeber this dataspace for checkpoint, if not already in list
		Dataspace_translation_info *trans_info = _dataspace_translations.first();
		if(trans_info) trans_info = trans_info->find_by_resto_badge(child_info->attached_ds_cap.local_name());
		if(!trans_info)
		{
			Ref_badge_info *badge_info = _region_maps.first();
			if(badge_info) badge_info = badge_info->find_by_badge(child_info->attached_ds_cap.local_name());
			if(!badge_info)
			{
				trans_info = new (_alloc) Dataspace_translation_info(
						stored_info->memory_content, child_info->attached_ds_cap, child_info->size);
				_dataspace_translations.insert(trans_info);
			}
		}

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_attached_region_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_addr(stored_info->rel_addr);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_attached_region(*stored_info);
		}

		stored_info = next_info;
	}
}
Stored_attached_region_info &Checkpointer::_create_stored_attached_region(Attached_region_info &child_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");


	// The dataspace with the memory content of the ram dataspace will be referenced by the stored ram dataspace
	Genode::Ram_dataspace_capability ramds_cap;

	// Exclude dataspaces which are known region maps (except managed dataspaces from the incremental checkpoint mechanism)
	Ref_badge_info *region_map_dataspace = _region_maps.first();
	if(region_map_dataspace) region_map_dataspace = region_map_dataspace->find_by_badge(child_info.attached_ds_cap.local_name());
	if(region_map_dataspace)
	{
		if(verbose_debug) Genode::log("Dataspace ", child_info.attached_ds_cap, " is a region map.");
	}
	else
	{
		// Check whether the dataspace is somewhere in the stored session RPC objects
		ramds_cap = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(
				_find_stored_dataspace(child_info.attached_ds_cap.local_name()));

		if(!ramds_cap.valid())
		{
			if(verbose_debug) Genode::log("Dataspace ", child_info.attached_ds_cap, " is not known. "
					"Creating dataspace with size ", Genode::Hex(child_info.size));
			ramds_cap = _state._env.ram().alloc(child_info.size);
		}
		else
		{
			if(verbose_debug) Genode::log("Dataspace ", child_info.attached_ds_cap, " is known from last checkpoint.");
		}
	}

	Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info.attached_ds_cap.local_name());
	return *new (_state._alloc) Stored_attached_region_info(child_info, childs_kcap, ramds_cap);
}
void Checkpointer::_destroy_stored_attached_region(Stored_attached_region_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	// Pre-condition: This stored object is removed from its list, thus,
	// a search for a stored dataspace will not return its memory content dataspace
	Genode::Dataspace_capability stored_ds_cap = _find_stored_dataspace(stored_info.attached_ds_badge);
	if(!stored_ds_cap.valid())
	{
		_state._env.ram().free(stored_info.memory_content);
	}

	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_ram_sessions(Genode::List<Stored_ram_session_info> &stored_infos,
		Genode::List<Ram_session_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Ram_session_component *child_info = nullptr;
	Stored_ram_session_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_state._alloc) Stored_ram_session_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		_prepare_ram_dataspaces(stored_info->stored_ramds_infos, child_info->parent_state().ram_dataspaces);

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_ram_session_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_ram_session(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_ram_session(Stored_ram_session_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Stored_ram_dataspace_info *info = stored_info.stored_ramds_infos.first())
	{
		stored_info.stored_ramds_infos.remove(info);
		_destroy_stored_ram_dataspace(*info);
	}
	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_ram_dataspaces(Genode::List<Stored_ram_dataspace_info> &stored_infos,
		Genode::List<Ram_dataspace_info> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Ram_dataspace_info *child_info = nullptr;
	Stored_ram_dataspace_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		Designated_redundant_ds_info* drdsi = nullptr;
		Designated_dataspace_info* dsi = child_info->mrm_info->dd_infos.first();
		if(dsi && dsi->redundant_memory)
		{
			drdsi = static_cast<Designated_redundant_ds_info*>(dsi);
		}


		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			if(drdsi && drdsi->redundant_writing())
				stored_info = &_create_stored_ram_dataspace(*child_info, &drdsi->get_first_checkpoint()->red_ds_cap);
			else
				stored_info = &_create_stored_ram_dataspace(*child_info);
			stored_infos.insert(stored_info);
		}

		// Nothing to update in stored_info

		//TODO: dont remember for redundant memory
		// Remeber this dataspace for checkpoint, if not already in list
		Dataspace_translation_info *trans_info = _dataspace_translations.first();
		if(trans_info) trans_info = trans_info->find_by_resto_badge(child_info->cap.local_name());
		if(!trans_info)
		{
			trans_info = new (_alloc) Dataspace_translation_info(
					stored_info->memory_content, child_info->cap, child_info->size);
			_dataspace_translations.insert(trans_info);
		}

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_ram_dataspace_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_ram_dataspace(*stored_info);
		}

		stored_info = next_info;
	}
}



Stored_ram_dataspace_info &Checkpointer::_create_stored_ram_dataspace(Ram_dataspace_info &child_info,
		const Genode::Dataspace_capability* red_mem_ds)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	// The dataspace with the memory content of the ram dataspace will be referenced by the stored ram dataspace
	Genode::Ram_dataspace_capability ramds_cap;

	// Exclude dataspaces which are known region maps (except managed dataspaces from the incremental checkpoint mechanism)
	Ref_badge_info *region_map_dataspace = _region_maps.first();
	if(region_map_dataspace) region_map_dataspace = region_map_dataspace->find_by_badge(child_info.cap.local_name());
	if(region_map_dataspace)
	{
		if(verbose_debug) Genode::log("Dataspace ", child_info.cap, " is a region map.");
	}
	else
	{
		// Check whether the dataspace is somewhere in the stored session RPC objects
		ramds_cap = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(
				_find_stored_dataspace(child_info.cap.local_name()));

		if(!ramds_cap.valid())
		{
			if(red_mem_ds)
			{
				if(verbose_debug) Genode::log("Dataspace ", child_info.cap, " has redundant memory. "
						"Linking with redundant memory dataspace ", *red_mem_ds);
				ramds_cap = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(*red_mem_ds);
			}
			else
			{
				if(verbose_debug) Genode::log("Dataspace ", child_info.cap, " is not known. "
						"Creating dataspace with size ", Genode::Hex(child_info.size));
				ramds_cap = _state._env.ram().alloc(child_info.size);
			}
		}
		else
		{
			if(verbose_debug) Genode::log("Dataspace ", child_info.cap, " is known from last checkpoint.");
		}
	}

	Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info.cap.local_name());
	return *new (_state._alloc) Stored_ram_dataspace_info(child_info, childs_kcap, ramds_cap);
}
void Checkpointer::_destroy_stored_ram_dataspace(Stored_ram_dataspace_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	// Pre-condition: This stored object is removed from its list, thus,
	// a search for a stored dataspace will not return its memory content dataspace
	Genode::Dataspace_capability stored_ds_cap = _find_stored_dataspace(stored_info.badge);
	if(!stored_ds_cap.valid())
	{
		_state._env.ram().free(stored_info.memory_content);
	}

	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_cpu_sessions(Genode::List<Stored_cpu_session_info> &stored_infos,
		Genode::List<Cpu_session_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Cpu_session_component *child_info = nullptr;
	Stored_cpu_session_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_state._alloc) Stored_cpu_session_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
		_prepare_cpu_threads(stored_info->stored_cpu_thread_infos, child_info->parent_state().cpu_threads);

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_cpu_session_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_cpu_session(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_cpu_session(Stored_cpu_session_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Stored_cpu_thread_info *info = stored_info.stored_cpu_thread_infos.first())
	{
		stored_info.stored_cpu_thread_infos.remove(info);
		_destroy_stored_cpu_thread(*info);
	}
	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_cpu_threads(Genode::List<Stored_cpu_thread_info> &stored_infos,
		Genode::List<Cpu_thread_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Cpu_thread_component *child_info = nullptr;
	Stored_cpu_thread_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_state._alloc) Stored_cpu_thread_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		stored_info->started = child_info->parent_state().started;
		stored_info->paused = child_info->parent_state().paused;
		stored_info->single_step = child_info->parent_state().single_step;
		stored_info->affinity = child_info->parent_state().affinity;
		stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
		// XXX does not guarantee to return the current thread registers
		stored_info->ts = Genode::Cpu_thread_client(child_info->parent_cap()).state();

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_cpu_thread_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_cpu_thread(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_cpu_thread(Stored_cpu_thread_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_pd_sessions(Genode::List<Stored_pd_session_info> &stored_infos,
		Genode::List<Pd_session_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Pd_session_component *child_info = nullptr;
	Stored_pd_session_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_pd_kcap  = _find_kcap_by_badge(child_info->cap().local_name());
			Genode::addr_t childs_add_kcap = _find_kcap_by_badge(child_info->address_space_component().cap().local_name());
			Genode::addr_t childs_sta_kcap = _find_kcap_by_badge(child_info->stack_area_component().cap().local_name());
			Genode::addr_t childs_lin_kcap = _find_kcap_by_badge(child_info->linker_area_component().cap().local_name());
			stored_info = new (_state._alloc) Stored_pd_session_info(*child_info,
					childs_pd_kcap, childs_add_kcap, childs_sta_kcap, childs_lin_kcap);
			stored_infos.insert(stored_info);
		}

		// Wrap Region_maps of child's and checkpointer's PD session in lists for reusing _prepare_region_maps
		// The linked list pointers of the three regions maps are usually not used gloablly
		Genode::List<Stored_region_map_info> temp_stored;
		temp_stored.insert(&stored_info->stored_linker_area);
		temp_stored.insert(&stored_info->stored_stack_area);
		temp_stored.insert(&stored_info->stored_address_space);
		Genode::List<Region_map_component> temp_child;
		temp_child.insert(&child_info->linker_area_component());
		temp_child.insert(&child_info->stack_area_component());
		temp_child.insert(&child_info->address_space_component());
		// Update stored_info
		_prepare_native_caps(stored_info->stored_native_cap_infos, child_info->parent_state().native_caps);
		_prepare_signal_sources(stored_info->stored_source_infos, child_info->parent_state().signal_sources);
		_prepare_signal_contexts(stored_info->stored_context_infos, child_info->parent_state().signal_contexts);
		_prepare_region_maps(temp_stored, temp_child);

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_pd_session_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_pd_session(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_pd_session(Stored_pd_session_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Stored_signal_context_info *info = stored_info.stored_context_infos.first())
	{
		stored_info.stored_context_infos.remove(info);
		_destroy_stored_signal_context(*info);
	}
	while(Stored_signal_source_info *info = stored_info.stored_source_infos.first())
	{
		stored_info.stored_source_infos.remove(info);
		_destroy_stored_signal_source(*info);
	}
	while(Stored_native_capability_info *info = stored_info.stored_native_cap_infos.first())
	{
		stored_info.stored_native_cap_infos.remove(info);
		_destroy_stored_native_cap(*info);
	}
	_destroy_stored_region_map(stored_info.stored_linker_area);
	_destroy_stored_region_map(stored_info.stored_stack_area);
	_destroy_stored_region_map(stored_info.stored_address_space);

	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_native_caps(Genode::List<Stored_native_capability_info> &stored_infos,
		Genode::List<Native_capability_info> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Native_capability_info *child_info = nullptr;
	Stored_native_capability_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap.local_name());
			stored_info = new (_state._alloc) Stored_native_capability_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Nothing to update in stored_info

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_native_capability_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_native_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_native_cap(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_native_cap(Stored_native_capability_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_signal_sources(Genode::List<Stored_signal_source_info> &stored_infos,
		Genode::List<Signal_source_info> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Signal_source_info *child_info = nullptr;
	Stored_signal_source_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap.local_name());
			stored_info = new (_state._alloc) Stored_signal_source_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Nothing to update in stored_info

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_signal_source_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_signal_source(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_signal_source(Stored_signal_source_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_signal_contexts(Genode::List<Stored_signal_context_info> &stored_infos,
		Genode::List<Signal_context_info> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Signal_context_info *child_info = nullptr;
	Stored_signal_context_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap.local_name());
			stored_info = new (_state._alloc) Stored_signal_context_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Nothing to update in stored_info

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_signal_context_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_signal_context(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_signal_context(Stored_signal_context_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_log_sessions(Genode::List<Stored_log_session_info> &stored_infos,
		Genode::List<Log_session_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Log_session_component *child_info = nullptr;
	Stored_log_session_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_state._alloc) Stored_log_session_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Nothing to update in stored_info

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_log_session_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_log_session(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_log_session(Stored_log_session_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::destroy(_state._alloc, &stored_info);
}


void Checkpointer::_prepare_timer_sessions(Genode::List<Stored_timer_session_info> &stored_infos,
		Genode::List<Timer_session_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Timer_session_component *child_info = nullptr;
	Stored_timer_session_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_state._alloc) Stored_timer_session_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
		stored_info->timeout = child_info->parent_state().timeout;
		stored_info->periodic = child_info->parent_state().periodic;

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_timer_session_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_timer_session(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_timer_session(Stored_timer_session_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::destroy(_state._alloc, &stored_info);
}

Genode::List<Ref_badge_info> Checkpointer::_create_region_map_dataspaces_list(
			Genode::List<Pd_session_component> &pd_sessions, Genode::List<Rm_session_component> *rm_sessions)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::List<Ref_badge_info> result_list;

	// Region maps of PD session
	Pd_session_component *pd_session = pd_sessions.first();
	while(pd_session)
	{
		Ref_badge_info *new_ref = nullptr;

		// Address space
		new_ref = new (_alloc) Ref_badge_info(pd_session->address_space_component().parent_state().ds_cap.local_name());
		result_list.insert(new_ref);

		// Stack area
		new_ref = new (_alloc) Ref_badge_info(pd_session->stack_area_component().parent_state().ds_cap.local_name());
		result_list.insert(new_ref);

		// Linker area
		new_ref = new (_alloc) Ref_badge_info(pd_session->linker_area_component().parent_state().ds_cap.local_name());
		result_list.insert(new_ref);

		pd_session = pd_session->next();
	}

	// Region maps of RM session, if there are any
	if(rm_sessions)
	{
		Rm_session_component *rm_session = rm_sessions->first();
		while(rm_session)
		{
			Region_map_component *region_map = rm_session->parent_state().region_maps.first();
			while(region_map)
			{
				Ref_badge_info *new_ref = new (_alloc) Ref_badge_info(region_map->parent_state().ds_cap.local_name());
				result_list.insert(new_ref);

				region_map = region_map->next();
			}

			rm_session = rm_session->next();
		}
	}

	return result_list;
}


void Checkpointer::_create_managed_dataspace_list(Genode::List<Ram_session_component> &ram_sessions)
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m(...)");

	typedef Simplified_managed_dataspace_info::Simplified_designated_ds_info Sim_dd_info;

	Simplified_managed_dataspace_info* last_elem = 0;

	Ram_session_component *ram_session = ram_sessions.first();
	while(ram_session)
	{
		Ram_dataspace_info *ramds_info = ram_session->parent_state().ram_dataspaces.first();
		while(ramds_info)
		{
			// RAM dataspace is managed
			if(ramds_info->mrm_info)
			{
				Genode::List<Sim_dd_info> sim_dd_infos;
				Designated_dataspace_info *dd_info = ramds_info->mrm_info->dd_infos.first();
				while(dd_info)
				{
					Genode::Ram_dataspace_capability dd_info_cap =
							Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(dd_info->cap);

					// Check if it's a redundant memory ds and pass a reference (which is otherwise NULL)
					Designated_redundant_ds_info* drdsi = nullptr;
					if(dd_info->redundant_memory)
						drdsi = static_cast<Designated_redundant_ds_info*>(dd_info);

					sim_dd_infos.insert(new (_alloc) Sim_dd_info(dd_info_cap, dd_info->rel_addr, dd_info->size, dd_info->attached, drdsi));

					dd_info = dd_info->next();
				}

				Simplified_managed_dataspace_info* new_elem = new (_alloc) Simplified_managed_dataspace_info(ramds_info->cap, sim_dd_infos);
				_managed_dataspaces->insert(new_elem, last_elem);
				last_elem = new_elem;
			}

			ramds_info = ramds_info->next();
		}

		ram_session = ram_session->next();
	}

}


void Checkpointer::_detach_designated_dataspaces(Genode::List<Ram_session_component> &ram_sessions,
		Genode::List<Designated_dataspace_info>* attached_dataspaces)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Ram_session_component *ram_session = ram_sessions.first();
	while(ram_session)
	{
		Ram_dataspace_info *ramds_info = ram_session->parent_state().ram_dataspaces.first();
		while(ramds_info)
		{
			if(ramds_info->mrm_info)
			{
				Designated_dataspace_info *dd_info = ramds_info->mrm_info->dd_infos.first();
				while(dd_info)
				{
					if(dd_info->attached)
					{
						dd_info->detach();
						// remember attached dataspaces for reattaching when using redundant memory
						if(attached_dataspaces)
							attached_dataspaces->insert(dd_info);
					}
					dd_info = dd_info->next();
				}
			}
			ramds_info = ramds_info->next();
		}
		ram_session = ram_session->next();
	}
}

void Checkpointer::_attach_designated_dataspaces(Genode::List<Designated_dataspace_info> ddis)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
	for(Designated_dataspace_info* ddi = ddis.first(); ddi != nullptr; ddi = ddi->next())
	{
		ddi->attach();
	}
}


void Checkpointer::_checkpoint_redundant_dataspaces(Genode::List<Ram_session_component> &ram_sessions)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Ram_session_component *ram_session = ram_sessions.first();
	while(ram_session)
	{
		Ram_dataspace_info *ramds_info = ram_session->parent_state().ram_dataspaces.first();
		while(ramds_info)
		{
			if(ramds_info->mrm_info)
			{
				Designated_redundant_ds_info *drds_info = (Designated_redundant_ds_info*) ramds_info->mrm_info->dd_infos.first();
				while(drds_info)
				{
					if(drds_info->redundant_writing())
						drds_info->trigger_new_checkpoint();
					drds_info = (Designated_redundant_ds_info*) drds_info->next();
				}
			}
			ramds_info = ramds_info->next();
		}
		ram_session = ram_session->next();
	}
}


void Checkpointer::_checkpoint_dataspaces()
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");


	Dataspace_translation_info *memory_info = _dataspace_translations.first();
	while(memory_info)
	{
		if(!memory_info->processed)
		{
			// Resolve managed dataspace of the incremental checkpointing mechanism
			Simplified_managed_dataspace_info *smd_info = _managed_dataspaces->first();
			if(smd_info) smd_info = smd_info->find_by_badge(memory_info->resto_ds_cap.local_name());
			// Dataspace is managed
			if(smd_info)
			{
				Simplified_managed_dataspace_info::Simplified_designated_ds_info *sdd_info =
						smd_info->designated_dataspaces.first();
				while(sdd_info)
				{
					if(sdd_info->modified)
					{
						PINF("mang");
						//TODO red mem	sdd_info->redundant_memory
						if(!sdd_info->redundant_memory)
							_checkpoint_dataspace_content(memory_info->ckpt_ds_cap, sdd_info->dataspace_cap, sdd_info->addr, sdd_info->size);
					}

					sdd_info = sdd_info->next();
				}

			}
			// Dataspace is not managed
			else
			{
				PINF("nonmang");
				_checkpoint_dataspace_content(memory_info->ckpt_ds_cap, memory_info->resto_ds_cap, 0, memory_info->size);
			}

			memory_info->processed = true;
		}

		memory_info = memory_info->next();
	}
}


void Checkpointer::_checkpoint_dataspace_content(Genode::Dataspace_capability dst_ds_cap,
		Genode::Dataspace_capability src_ds_cap, Genode::addr_t dst_offset, Genode::size_t size)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(dst ", dst_ds_cap,
			", src ", src_ds_cap, ", dst_offset=", Genode::Hex(dst_offset),
			", copy_size=", Genode::Hex(size), ")");

	char *dst_addr_start = _state._env.rm().attach(dst_ds_cap);
	char *src_addr_start = _state._env.rm().attach(src_ds_cap);

	Genode::memcpy(dst_addr_start + dst_offset, src_addr_start, size);

	_state._env.rm().detach(src_addr_start);
	_state._env.rm().detach(dst_addr_start);
}


Checkpointer::Checkpointer(Genode::Allocator &alloc, Target_child &child, Target_state &state)
:
	_alloc(alloc), _child(child), _state(state)
{
	if(verbose_debug) Genode::log("\033[33m", "Checkpointer", "\033[0m(...)");

//	if(_child.use_redundant_memory)
//		set_redundant_memory(true);
}

Checkpointer::~Checkpointer()
{
	if(verbose_debug) Genode::log("\033[33m", "~Checkpointer", "\033[0m");

	_destroy_list(_kcap_mappings);
	_destroy_list(_dataspace_translations);
	_destroy_list(_region_maps);
	if(_managed_dataspaces != nullptr)
		_destroy_list(*_managed_dataspaces);
}

void Checkpointer::set_redundant_memory(bool active)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...), ", active ? "ENABLE" : "DISABLE");

	//_detach_designated_dataspaces(_child.custom_services().ram_root->session_infos());
	// due to the incomplete Fiasco.OC register backups delivered to Genode,
	// a full instruction simulation and thus redundant memory of ALL dataspaces is
	// not possible. Otherwise, cnt would not be needed.
	Genode::size_t cnt = 0;
	for(Ram_dataspace_info* rdsi = _child.ram().parent_state().ram_dataspaces.first();
			rdsi != nullptr && cnt < 1; rdsi = rdsi->next(), cnt++)
	{
		for(Designated_redundant_ds_info* drdsi =
				(Designated_redundant_ds_info*) rdsi->mrm_info->dd_infos.first();
				drdsi != nullptr;
				drdsi = (Designated_redundant_ds_info*) drdsi->next())
		{
			drdsi->redundant_writing(active);
		}
	}
}

void Checkpointer::_lock_redundant_dataspaces(bool lock)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...), ", lock ? "LOCK" : "UNLOCK");

	for(Ram_dataspace_info* rdsi = _child.ram().parent_state().ram_dataspaces.first();
			rdsi != nullptr; rdsi = rdsi->next())
	{
		for(Designated_redundant_ds_info* drdsi =
				(Designated_redundant_ds_info*) rdsi->mrm_info->dd_infos.first();
				drdsi != nullptr;
				drdsi = (Designated_redundant_ds_info*) drdsi->next())
		{
			if(drdsi->redundant_writing())
			{
				if(lock)
					drdsi->lock();
				else
					drdsi->unlock();
			}
		}
	}
}



void Checkpointer::checkpoint()
{
	using Genode::log;
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");

	_managed_dataspaces = &_state._managed_redundant_dataspaces;

	if(_child.use_redundant_memory)
	{
		_destroy_list(*_managed_dataspaces);
		_lock_redundant_dataspaces(true);
	}

	// Pause child
	_child.pause();

	// Create mapping of badge to kcap
	_kcap_mappings = _create_kcap_mappings();

	if(verbose_debug)
	{
		Genode::log("Capability map:");
		Kcap_badge_info const *info = _kcap_mappings.first();
		if(!info) Genode::log(" <empty>\n");
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Create a list of region map dataspaces which are known to child
	// These dataspaces are ignored when creating copy dataspaces
	// For new intercepted sessions which trade managed dataspaces between child and themselves,
	// the region map dataspace capability has to be inserted into this list
	Genode::List<Rm_session_component> *rm_sessions = nullptr;
	if(_child.custom_services().rm_root) rm_sessions = &_child.custom_services().rm_root->session_infos();
	_region_maps = _create_region_map_dataspaces_list(_child.custom_services().pd_root->session_infos(), rm_sessions);

	if(verbose_debug)
	{
		Genode::log("Region map dataspaces:");
		Ref_badge_info const *info = _region_maps.first();
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Prepare state lists
	// implicitly _copy_dataspaces modified with the child's currently known dataspaces and copy dataspaces
	_prepare_ram_sessions(_state._stored_ram_sessions, _child.custom_services().ram_root->session_infos());
	_prepare_pd_sessions(_state._stored_pd_sessions, _child.custom_services().pd_root->session_infos());
	_prepare_cpu_sessions(_state._stored_cpu_sessions, _child.custom_services().cpu_root->session_infos());
	if(_child.custom_services().rm_root)
		_prepare_rm_sessions(_state._stored_rm_sessions, _child.custom_services().rm_root->session_infos());
	if(_child.custom_services().log_root)
		_prepare_log_sessions(_state._stored_log_sessions, _child.custom_services().log_root->session_infos());
	if(_child.custom_services().timer_root)
		_prepare_timer_sessions(_state._stored_timer_sessions, _child.custom_services().timer_root->session_infos());

	if(verbose_debug)
	{
		Genode::log("Dataspaces to checkpoint:");
		Dataspace_translation_info *info = _dataspace_translations.first();
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Create a list of managed dataspaces
	_create_managed_dataspace_list(_child.custom_services().ram_root->session_infos());

	if(verbose_debug)
	{
		Genode::log("Managed dataspaces:");
		Simplified_managed_dataspace_info const *smd_info = _managed_dataspaces->first();
		if(!smd_info) Genode::log(" <empty>\n");
		while(smd_info)
		{
			Genode::log(" ", *smd_info);

			Simplified_managed_dataspace_info::Simplified_designated_ds_info const *sdd_info =
					smd_info->designated_dataspaces.first();
			if(!sdd_info) Genode::log("  <empty>\n");
			while(sdd_info)
			{
				Genode::log("  ", *sdd_info);

				sdd_info = sdd_info->next();
			}

			smd_info = smd_info->next();
		}
	}

	Genode::List<Designated_dataspace_info> attached_dataspaces;

	_detach_designated_dataspaces(_child.custom_services().ram_root->session_infos(), &attached_dataspaces);
	// Copy child dataspaces' content and to stored dataspaces' content
	_checkpoint_dataspaces();

	if(_child.use_redundant_memory)
	{
		_attach_designated_dataspaces(attached_dataspaces);

		// Redirect redundant writes to a fresh set of dataspaces
		_checkpoint_redundant_dataspaces(_child.custom_services().ram_root->session_infos());
	}


	if(verbose_debug) Genode::log(_child);
	if(verbose_debug) Genode::log(_state);

	// Clean up
	_destroy_list(_kcap_mappings);
	_destroy_list(_dataspace_translations);
	_destroy_list(_region_maps);
	if(!_child.use_redundant_memory)
		_destroy_list(*_managed_dataspaces);

	// Resume child
	_child.resume();

	if(_child.use_redundant_memory)
		_lock_redundant_dataspaces(false);

}
