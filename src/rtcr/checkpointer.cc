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


Genode::List<Badge_kcap_info> Checkpointer::_create_cap_map_infos()
{
	using Genode::log;
	using Genode::Hex;
	using Genode::addr_t;
	using Genode::size_t;
	using Genode::uint16_t;

	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");

	Genode::List<Badge_kcap_info> result;

	// Retrieve cap_idx_alloc_addr
	Genode::addr_t cap_idx_alloc_addr = Genode::Foc_native_pd_client(_child.pd().native_pd()).cap_map_info();
	//log("from ckpt: ", Genode::Hex(cap_idx_alloc_addr));

	// Find child's dataspace corresponding to cap_idx_alloc_addr
	Attached_region_info *ar_info = _child.pd().address_space_component().parent_state().attached_regions.first();
	if(ar_info) ar_info = ar_info->find_by_addr(cap_idx_alloc_addr);
	if(!ar_info)
	{
		Genode::error("No dataspace found for cap_idx_alloc's datastructure at ", Hex(cap_idx_alloc_addr));
		throw Genode::Exception();
	}

	// If ar_info is a managed Ram_dataspace_info, mark detached Designated_dataspace_infos and attach them,
	// thus, the Checkpointer does not trigger page faults which mark accessed regions
	Genode::List<Ref_badge> marked_badge_infos =
			_mark_attach_designated_dataspaces(*ar_info);

	// Create new badge_kcap list
	//const Genode::size_t struct_size    = sizeof(Genode::Cap_index_allocator_tpl<Genode::Cap_index,4096>);
	const size_t array_ele_size = sizeof(Genode::Cap_index);
	const size_t array_size     = array_ele_size*4096;

	const addr_t child_ds_start     = ar_info->rel_addr;
	//const addr_t child_ds_end       = child_ds_start + ar_info->size;
	//const addr_t child_struct_start = cap_idx_alloc_addr;
	//const addr_t child_struct_end   = child_struct_start + struct_size;
	//const addr_t child_array_start  = child_struct_start + 8;
	//const addr_t child_array_end    = child_array_start + array_size;

	const addr_t local_ds_start     = _state._env.rm().attach(ar_info->attached_ds_cap, ar_info->size, ar_info->offset);
	//const addr_t local_ds_end       = local_ds_start + ar_info->size;
	const addr_t local_struct_start = cap_idx_alloc_addr - child_ds_start + local_ds_start;
	//const addr_t local_struct_end   = local_struct_start + struct_size;
	const addr_t local_array_start  = local_struct_start + 8;
	const addr_t local_array_end    = local_array_start + array_size;

	/*
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
	*/

	//dump_mem((void*)local_array_start, 0x1200);

	for(addr_t curr = local_array_start; curr < local_array_end; curr += array_ele_size)
	{
/*
		log("  ",
				Hex(*(Genode::uint8_t*)(curr+0), Hex::OMIT_PREFIX, Hex::PAD),
				Hex(*(Genode::uint8_t*)(curr+1), Hex::OMIT_PREFIX, Hex::PAD),
				Hex(*(Genode::uint8_t*)(curr+2), Hex::OMIT_PREFIX, Hex::PAD),
				Hex(*(Genode::uint8_t*)(curr+3), Hex::OMIT_PREFIX, Hex::PAD), "  ",
				Hex(*(Genode::uint8_t*)(curr+4), Hex::OMIT_PREFIX, Hex::PAD),
				Hex(*(Genode::uint8_t*)(curr+5), Hex::OMIT_PREFIX, Hex::PAD),
				Hex(*(Genode::uint8_t*)(curr+6), Hex::OMIT_PREFIX, Hex::PAD),
				Hex(*(Genode::uint8_t*)(curr+7), Hex::OMIT_PREFIX, Hex::PAD));
*/
		const size_t badge_offset = 6;

		// Convert address to pointer and dereference it
		const uint16_t badge = *(uint16_t*)(curr + badge_offset);
		// Current capability map slot = Compute distance from start to current address of array and
		// divide it by the element size;
		// kcap = current capability map slot shifted by 12 bits to the left (last 12 bits are used
		// by Fiasco.OC for parameters for IPC calls)
		const addr_t kcap  = ((curr - local_array_start) / array_ele_size) << 12;

		if(badge != 0 && badge != 0xffff)
		{
			Badge_kcap_info *state_info = new (_alloc) Badge_kcap_info(kcap, badge);
			result.insert(state_info);
			//log("+ ", Hex(kcap), ": ", badge, " (", Hex(badge), ")");
		}
		else
		{
			//log("  ", Hex(kcap), ": ", badge);
		}


	}

	// Detach the previously attached Designated_dataspace_infos and delete the list containing marked Designated_dataspace_infos
	_detach_unmark_designated_dataspaces(marked_badge_infos, *ar_info);

	return result;
}


void Checkpointer::_destroy_cap_map_infos(Genode::List<Badge_kcap_info> &cap_map_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Badge_kcap_info *info = cap_map_infos.first())
	{
		cap_map_infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}


Genode::List<Ref_badge> Checkpointer::_mark_attach_designated_dataspaces(Attached_region_info &ar_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::List<Ref_badge> result_infos;

	Managed_region_map_info *mrm_info = ar_info.managed_dataspace(_child.ram().parent_state().ram_dataspaces);
	if(mrm_info)
	{
		Designated_dataspace_info *dd_info = mrm_info->dd_infos.first();
		while(dd_info)
		{
			if(!dd_info->attached)
			{
				dd_info->attach();

				Ref_badge *new_info = new (_alloc) Ref_badge(dd_info->cap.local_name());
				result_infos.insert(new_info);
			}

			dd_info = dd_info->next();
		}
	}

	return result_infos;
}


void Checkpointer::_detach_unmark_designated_dataspaces(Genode::List<Ref_badge> &badge_infos, Attached_region_info &ar_info)
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
	while(Ref_badge *badge_info = badge_infos.first())
	{
		badge_infos.remove(badge_info);
		Genode::destroy(_alloc, badge_info);
	}
}


Genode::addr_t Checkpointer::_find_kcap_by_badge(Genode::uint16_t badge, Genode::List<Badge_kcap_info> &cap_map_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::addr_t kcap = 0;

	Badge_kcap_info *info = cap_map_infos.first();
	if(info) info = info->find_by_badge(badge);
	if(info) kcap = info->kcap;

	return kcap;
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name(), _capability_map_infos);
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
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name(), _capability_map_infos);
			stored_info = new (_state._alloc) Stored_region_map_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
		_prepare_attached_regions(stored_info->stored_attached_region_infos, child_info->parent_state().attached_regions);

		// Remeber region map's dataspace badge to remove the dataspace from memory to checkpoint later
		Ref_badge *ref_badge = new (_alloc) Ref_badge(child_info->parent_state().ds_cap.local_name());
		_region_map_dataspaces.insert(ref_badge);

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

	// Find attached ds cap
	Genode::Ram_dataspace_capability ramds_cap;
	Orig_copy_count_info *known_info = _copy_dataspaces.first();
	if(known_info) known_info = known_info->find_by_badge(child_info.attached_ds_cap.local_name());
	if(known_info)
	{
		if(verbose_debug) Genode::log("Dataspace ", child_info.attached_ds_cap, " is already known.");

		ramds_cap = known_info->copy_ds_cap;
		known_info->ref_count++;
	}
	else
	{
		// Exclude dataspaces which are known region maps (except inc ckpt dataspaces)
		Ref_badge *region_map_dataspace = _region_map_dataspaces.first();
		if(region_map_dataspace) region_map_dataspace = region_map_dataspace->find_by_badge(child_info.attached_ds_cap.local_name());
		if(!region_map_dataspace)
		{
			if(verbose_debug) Genode::log("Dataspace ", child_info.attached_ds_cap, " is not known. "
					"Creating dataspace with size ", Genode::Hex(child_info.size));
			ramds_cap = _state._env.ram().alloc(child_info.size);
			known_info = new (_alloc) Orig_copy_count_info(child_info.attached_ds_cap, ramds_cap, child_info.size);
			_copy_dataspaces.insert(known_info);
		}
		else
		{
			if(verbose_debug) Genode::log("Dataspace ", child_info.attached_ds_cap, " is a region map");
		}
	}

	return *new (_state._alloc) Stored_attached_region_info(child_info, ramds_cap);
}
void Checkpointer::_destroy_stored_attached_region(Stored_attached_region_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	// Decrement ref_cound of corresponding known_dataspace entry and delete it, if necessary
	Orig_copy_count_info *known_info =_copy_dataspaces.first();
	if(known_info) known_info = known_info->find_by_badge(stored_info.attached_ds_badge);
	if(known_info)
	{
		known_info->ref_count--;
		if(known_info->ref_count < 1)
		{
			_copy_dataspaces.remove(known_info);
			Genode::destroy(_alloc, known_info);
		}
	}
	else
	{
		Genode::error("No entry in _copy_dataspaces for ", stored_info.attached_ds_badge);
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name(), _capability_map_infos);
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
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			stored_info = &_create_stored_ram_dataspace(*child_info);
			stored_infos.insert(stored_info);
		}

		// Nothing to update in stored_info

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
Stored_ram_dataspace_info &Checkpointer::_create_stored_ram_dataspace(Ram_dataspace_info &child_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	// Create orignal_copy dataspace mapping (which is used to checkpoint memory content)
	Genode::Ram_dataspace_capability ramds_cap;
	Orig_copy_count_info *known_info = _copy_dataspaces.first();
	if(known_info) known_info = known_info->find_by_badge(child_info.cap.local_name());
	if(known_info)
	{
		if(verbose_debug) Genode::log("Dataspace ", child_info.cap, " is already known.");

		ramds_cap = known_info->copy_ds_cap;
		known_info->ref_count++;
	}
	else
	{
		if(verbose_debug) Genode::log("Dataspace ", child_info.cap, " is not known. "
			"Creating dataspace with size ", Genode::Hex(child_info.size));

		ramds_cap = _state._env.ram().alloc(child_info.size);
		known_info = new (_alloc) Orig_copy_count_info(child_info.cap, ramds_cap, child_info.size);
		_copy_dataspaces.insert(known_info);
	}

	// Find childs_kcap
	Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info.cap.local_name(), _capability_map_infos);

	return *new (_state._alloc) Stored_ram_dataspace_info(child_info, childs_kcap, ramds_cap);
}
void Checkpointer::_destroy_stored_ram_dataspace(Stored_ram_dataspace_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	// Decrement ref_cound of corresponding known_dataspace entry and delete it, if necessary
	Orig_copy_count_info *known_info =_copy_dataspaces.first();
	if(known_info) known_info = known_info->find_by_badge(stored_info.badge);
	if(known_info)
	{
		known_info->ref_count--;
		if(known_info->ref_count < 1)
		{
			_copy_dataspaces.remove(known_info);
			Genode::destroy(_alloc, known_info);
		}
	}
	else
	{
		Genode::error("No entry in _copy_dataspaces for ", stored_info.badge);
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name(), _capability_map_infos);
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name(), _capability_map_infos);
			stored_info = new (_state._alloc) Stored_cpu_thread_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		stored_info->started = child_info->parent_state().started;
		stored_info->paused = child_info->parent_state().paused;
		stored_info->single_step = child_info->parent_state().single_step;
		stored_info->affinity = child_info->parent_state().affinity;
		stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
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
		Genode::log(child_info->parent_state());
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_pd_kcap = _find_kcap_by_badge(child_info->cap().local_name(), _capability_map_infos);
			Genode::addr_t childs_add_kcap = _find_kcap_by_badge(child_info->address_space_component().cap().local_name(), _capability_map_infos);
			Genode::addr_t childs_sta_kcap = _find_kcap_by_badge(child_info->stack_area_component().cap().local_name(), _capability_map_infos);
			Genode::addr_t childs_lin_kcap = _find_kcap_by_badge(child_info->linker_area_component().cap().local_name(), _capability_map_infos);
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap.local_name(), _capability_map_infos);
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap.local_name(), _capability_map_infos);
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap.local_name(), _capability_map_infos);
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name(), _capability_map_infos);
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
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name(), _capability_map_infos);
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

Genode::List<Ref_badge> Checkpointer::_create_region_map_dataspaces(
			Genode::List<Pd_session_component> &pd_sessions, Genode::List<Rm_session_component> *rm_sessions)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::List<Ref_badge> result_list;

	// Region maps of PD session
	Pd_session_component *pd_session = pd_sessions.first();
	while(pd_session)
	{
		Ref_badge *new_ref = nullptr;

		// Address space
		new_ref = new (_alloc) Ref_badge(pd_session->address_space_component().parent_state().ds_cap.local_name());
		result_list.insert(new_ref);

		// Stack area
		new_ref = new (_alloc) Ref_badge(pd_session->stack_area_component().parent_state().ds_cap.local_name());
		result_list.insert(new_ref);

		// Linker area
		new_ref = new (_alloc) Ref_badge(pd_session->linker_area_component().parent_state().ds_cap.local_name());
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
				Ref_badge *new_ref = new (_alloc) Ref_badge(region_map->parent_state().ds_cap.local_name());
				result_list.insert(new_ref);

				region_map = region_map->next();
			}

			rm_session = rm_session->next();
		}
	}

	return result_list;
}


Genode::List<Orig_copy_ckpt_info> Checkpointer::_create_memory_to_checkpoint(Genode::List<Orig_copy_count_info> &copy_dataspaces)
{
	Genode::List<Orig_copy_ckpt_info> result_list;

	Orig_copy_count_info *occ_info = copy_dataspaces.first();
	while(occ_info)
	{
		Orig_copy_ckpt_info *new_info = new (_alloc) Orig_copy_ckpt_info(occ_info->orig_ds_cap, occ_info->copy_ds_cap,
				0, occ_info->size);
		result_list.insert(new_info);

		occ_info = occ_info->next();
	}

	return result_list;
}


void Checkpointer::_resolve_inc_checkpoint_dataspaces(
		Genode::List<Ram_session_component> &ram_sessions, Genode::List<Orig_copy_ckpt_info> &memory_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

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
				Orig_copy_ckpt_info *memory_info = memory_infos.first();
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
						Orig_copy_ckpt_info *new_oc_info = new (_alloc) Orig_copy_ckpt_info(dd_info->cap,
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


void Checkpointer::_detach_designated_dataspaces(Genode::List<Ram_session_component> &ram_sessions)
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
					if(dd_info->attached) dd_info->detach();
					dd_info = dd_info->next();
				}
			}
			ramds_info = ramds_info->next();
		}
		ram_session = ram_session->next();
	}
}


void Checkpointer::_destroy_memory_to_checkpoint(Genode::List<Orig_copy_ckpt_info> &memory_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Orig_copy_ckpt_info *info = memory_infos.first())
	{
		memory_infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}


void Checkpointer::_destroy_region_map_dataspaces(Genode::List<Ref_badge> &mands_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Ref_badge *info = mands_infos.first())
	{
		mands_infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}


void Checkpointer::_destroy_copy_dataspaces(Genode::List<Orig_copy_count_info> known_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	while(Orig_copy_count_info *info = known_infos.first())
	{
		known_infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}


void Checkpointer::_checkpoint_dataspaces(Genode::List<Orig_copy_ckpt_info> &memory_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Orig_copy_ckpt_info *memory_info = memory_infos.first();
	while(memory_info)
	{
		if(!memory_info->checkpointed)
		{
			_checkpoint_dataspace_content(memory_info->orig_ds_cap, memory_info->copy_ds_cap,
					memory_info->copy_rel_addr, memory_info->copy_size);
			memory_info->checkpointed = true;
		}

		memory_info = memory_info->next();
	}
}


void Checkpointer::_checkpoint_dataspace_content(Genode::Dataspace_capability orig_ds_cap,
		Genode::Ram_dataspace_capability copy_ds_cap, Genode::addr_t copy_rel_addr, Genode::size_t copy_size)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(orig ", orig_ds_cap,
			", copy ", copy_ds_cap, ", copy_rel_addr=", Genode::Hex(copy_rel_addr),
			", copy_size=", Genode::Hex(copy_size), ")");

	char *orig = _state._env.rm().attach(orig_ds_cap);
	char *copy = _state._env.rm().attach(copy_ds_cap);

	Genode::memcpy(copy + copy_rel_addr, orig, copy_size);

	_state._env.rm().detach(copy);
	_state._env.rm().detach(orig);
}


Checkpointer::Checkpointer(Genode::Allocator &alloc, Target_child &child, Target_state &state)
:
	_alloc(alloc), _child(child), _state(state)
{
	if(verbose_debug) Genode::log("\033[33m", "Checkpointer", "\033[0m(...)");
}

Checkpointer::~Checkpointer()
{
	if(verbose_debug) Genode::log("\033[33m", "~Checkpointer", "\033[0m");

	_destroy_cap_map_infos(_capability_map_infos);
	_destroy_memory_to_checkpoint(_memory_to_checkpoint);
	_destroy_region_map_dataspaces(_region_map_dataspaces);
	_destroy_copy_dataspaces(_copy_dataspaces);
}


void Checkpointer::checkpoint()
{
	using Genode::log;
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");

	// Pause child
	_child.pause();

	// Create mapping of badge to kcap
	_capability_map_infos = _create_cap_map_infos();

	if(verbose_debug)
	{
		Genode::log("Capability map:");
		Badge_kcap_info const *info = _capability_map_infos.first();
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
	_region_map_dataspaces = _create_region_map_dataspaces(_child.custom_services().pd_root->session_infos(), rm_sessions);

	if(verbose_debug)
	{
		Genode::log("Region map dataspaces:");
		Ref_badge const *info = _region_map_dataspaces.first();
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Prepare state lists
	// implicitly _copy_dataspaces modified with the child's currently known dataspaces and copy dataspaces
	_prepare_pd_sessions(_state._stored_pd_sessions, _child.custom_services().pd_root->session_infos());
	_prepare_cpu_sessions(_state._stored_cpu_sessions, _child.custom_services().cpu_root->session_infos());
	_prepare_ram_sessions(_state._stored_ram_sessions, _child.custom_services().ram_root->session_infos());
	if(_child.custom_services().rm_root)
		_prepare_rm_sessions(_state._stored_rm_sessions, _child.custom_services().rm_root->session_infos());
	if(_child.custom_services().log_root)
		_prepare_log_sessions(_state._stored_log_sessions, _child.custom_services().log_root->session_infos());
	if(_child.custom_services().timer_root)
		_prepare_timer_sessions(_state._stored_timer_sessions, _child.custom_services().timer_root->session_infos());

	if(verbose_debug)
	{
		Genode::log("Copy dataspaces");
		Orig_copy_count_info *info = _copy_dataspaces.first();
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Create list of dataspace capabilities which will be checkpointed in a separate phase
	_memory_to_checkpoint = _create_memory_to_checkpoint(_copy_dataspaces);

	// Resolve managed dataspaces from incremental checkpoint to simple dataspaces in memory_to_checkpoint
	_resolve_inc_checkpoint_dataspaces(_child.custom_services().ram_root->session_infos(), _memory_to_checkpoint);

	if(verbose_debug)
	{
		Genode::log("Memory to checkpoint");
		Orig_copy_ckpt_info *info = _memory_to_checkpoint.first();
		while(info)
		{
			Genode::log(" ", *info);
			info = info->next();
		}
	}

	// Detach all designated dataspaces
	_detach_designated_dataspaces(_child.custom_services().ram_root->session_infos());

	// Checkpoint memory in memory_to_checkpoint
	_checkpoint_dataspaces(_memory_to_checkpoint);

	if(verbose_debug) Genode::log(_child);
	if(verbose_debug) Genode::log(_state);

	// Destroy list elements
	_destroy_cap_map_infos(_capability_map_infos);
	_destroy_memory_to_checkpoint(_memory_to_checkpoint);
	_destroy_region_map_dataspaces(_region_map_dataspaces);

	// Resume child
	//_child.resume();
}
