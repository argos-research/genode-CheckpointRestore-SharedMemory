/*
 * \brief  Child copy
 * \author Denis Huber
 * \date   2016-09-07
 */

#include "target_copy.h"
#include "util/debug.h"

using namespace Rtcr;


void Target_copy::_copy_threads()
{
	Thread_info *curr_th = _threads.first();
	for( ; curr_th; curr_th = curr_th->next())
	{
		Thread_info *new_th = new (_alloc) Thread_info(curr_th->thread_cap);

		_copied_threads.insert(new_th);
	}
}


void Target_copy::_copy_capabilities()
{
	Genode::log(__func__, ": Implement me!");
}


void Target_copy::_copy_region_maps()
{
	// Copy Copy_region_infos of stack area
	if(verbose_debug) Genode::log("  copying stack area");
	_copy_region_map(_copied_stack_regions, _stack_regions);

	// Copy Copy_region_infos of linker area
	if(verbose_debug) Genode::log("  copying linker area");
	_copy_region_map(_copied_linker_regions, _linker_regions);

	// Copy Copy_region_infos of address space
	if(verbose_debug) Genode::log("  copying address space");
	_copy_region_map(_copied_address_space_regions, _address_space_regions);
}


void Target_copy::_copy_region_map(Genode::List<Copied_region_info> &copy_infos, Genode::List<Attached_region_info> &orig_infos)
{
	// Delete each Copied_region_info, if the corresponding Attached_region_info is gone
	if(verbose_debug) Genode::log("    delete unused copy_region_infos");
	_delete_copied_region_infos(copy_infos, orig_infos);

	// Create each Copied_region_info, if a new Attached_region_info is found
	if(verbose_debug) Genode::log("    create new copy_region_infos");
	_create_copied_region_infos(copy_infos, orig_infos);

	// Copy dataspaces
	if(verbose_debug) Genode::log("    copy dataspaces");
	_copy_dataspaces(copy_infos, orig_infos);
}


void Target_copy::_delete_copied_region_infos(Genode::List<Copied_region_info> &copy_infos,
		Genode::List<Attached_region_info> &orig_infos)
{
	// Iterate through copy_infos and delete all Copied_region_infos which have no corresponding Attached_region_info
	Copied_region_info *copy_info = copy_infos.first();

	while(copy_info)
	{
		Copied_region_info *next = copy_info->next();

		Attached_region_info *orig_info = orig_infos.first();
		if(orig_info) orig_info = orig_info->find_by_cap_and_addr(copy_info->orig_ds_cap, copy_info->rel_addr);

		if(!orig_info)
		{
			// Delete Copied_region_info
			_delete_copied_region_info(*copy_info, copy_infos);
		}

		copy_info = next;
	}
}


void Target_copy::_delete_copied_region_info(Copied_region_info &info,
		Genode::List<Copied_region_info> &infos)
{
	// Free ram dataspace, if it is not shared between other Copied_region_infos
	Copied_region_info *other_info = infos.first()->find_by_copy_cap_and_not_addr(info.copy_ds_cap, info.rel_addr);
	if(!other_info)
	{
		_env.ram().free(Genode::static_cap_cast<Genode::Ram_dataspace>(info.copy_ds_cap));
	}

	infos.remove(&info);
	Genode::destroy(_alloc, &info);
}


void Target_copy::_create_copied_region_infos(Genode::List<Copied_region_info> &copy_infos,
		Genode::List<Attached_region_info> &orig_infos)
{
	for(Attached_region_info *orig_info = orig_infos.first(); orig_info; orig_info = orig_info->next())
	{
		// Skip stack and linker area which are attached in address space
		if(orig_info->ds_cap == _stack_ds_cap || orig_info->ds_cap == _linker_ds_cap)
			continue;

		Copied_region_info *copy_info = copy_infos.first();
		if(copy_info) copy_info = copy_info->find_by_orig_cap_and_addr(orig_info->ds_cap, orig_info->rel_addr);

		if(!copy_info)
		{
			// Create a corresponding Copied_region_info
			_create_copied_region_info(*orig_info, copy_infos);
		}
	}
}


void Target_copy::_create_copied_region_info(Attached_region_info &orig_info,
		Genode::List<Copied_region_info> &copy_infos)
{
	/*
	 *  Allocate new dataspace, if orig_info's dataspace is not found in copy_infos,
	 *  else reuse copy_info's copy_ds_cap
	 */
	Genode::Ram_dataspace_capability copy_ds_cap;
	Copied_region_info *copy_info = copy_infos.first();
	if(copy_info) copy_info = copy_info->find_by_orig_ds_cap(orig_info.ds_cap);
	if(copy_info)
	{
		copy_ds_cap = Genode::static_cap_cast<Genode::Ram_dataspace>(copy_info->copy_ds_cap);
	}
	else
	{
		copy_ds_cap = _env.ram().alloc(orig_info.size);
	}

	// Determine whether orig_info's dataspace is managed
	bool managed = orig_info.managed_dataspace(_ram_dataspace_infos) != nullptr;

	// Create and insert Copy_region_info
	Copied_region_info *new_copy_info = new (_alloc) Copied_region_info(orig_info, copy_ds_cap, managed);
	copy_infos.insert(new_copy_info);
}


void Target_copy::_copy_dataspaces(Genode::List<Copied_region_info> &copy_infos,
		Genode::List<Attached_region_info> &orig_infos)
{
	/*
	 * Iterate through copy_infos; because all copy_infos have to be filled.
	 * Wherelse not all orig_infos have to be copied: Stack and linker area.
	 */
	for(Copied_region_info *copy_info = copy_infos.first(); copy_info; copy_info = copy_info->next())
	{
		// Find corresponding copy_info
		Attached_region_info *orig_info =
				orig_infos.first()->find_by_cap_and_addr(copy_info->orig_ds_cap, copy_info->rel_addr);
		if(!orig_info)
		{
			Genode::error("No corresponding Attached_region_info for Copied_region_info ",
					copy_info->orig_ds_cap, " -> ", copy_info->copy_ds_cap);
			return;
		}

		// Determine whether orig_info's dataspace is managed
		Managed_region_map_info *mrm_info = orig_info->managed_dataspace(_ram_dataspace_infos);

		if(mrm_info)
		{
			// Managed: Copy only marked dataspaces
			_copy_managed_dataspace(*mrm_info, *copy_info);
		}
		else
		{
			// Not managed: Copy whole dataspaces
			_copy_dataspace(orig_info->ds_cap, copy_info->copy_ds_cap, orig_info->size);
		}
	}
}


void Target_copy::_copy_managed_dataspace(Managed_region_map_info &mrm_info, Copied_region_info &copy_info)
{
	// Iterate through all designated dataspaces
	Designated_dataspace_info *dd_info = mrm_info.dd_infos.first();
	while(dd_info)
	{
		// Copy content of marked dataspaces and unmark the designated dataspace
		if(dd_info->attached)
		{
			_copy_dataspace(dd_info->ds_cap, copy_info.copy_ds_cap, dd_info->size, dd_info->rel_addr);

			dd_info->detach();
		}

		dd_info = dd_info->next();
	}
}


void Target_copy::_copy_dataspace(Genode::Dataspace_capability &source_ds_cap, Genode::Dataspace_capability &dest_ds_cap,
		Genode::size_t size, Genode::off_t dest_offset)
{
	char *source = _env.rm().attach(source_ds_cap);
	char *dest   = _env.rm().attach(dest_ds_cap);

	Genode::memcpy(dest + dest_offset, source, size);

	_env.rm().detach(dest);
	_env.rm().detach(source);
}


Target_copy::Target_copy(Genode::Env &env, Genode::Allocator &alloc, Target_child &child)
:
	_env                         (env),
	_alloc                       (alloc),
	_threads                     (child.cpu().thread_infos()),
	_address_space_regions       (child.pd().address_space_component().attached_regions()),
	_stack_regions               (child.pd().stack_area_component().attached_regions()),
	_linker_regions              (child.pd().linker_area_component().attached_regions()),
	_ram_dataspace_infos         (child.ram().ram_dataspace_infos()),
	_copy_lock                   (),
	_copied_threads              (),
	_copied_address_space_regions(),
	_copied_stack_regions        (),
	_copied_linker_regions       (),
	_stack_ds_cap                (child.pd().stack_area_component().dataspace()),
	_linker_ds_cap               (child.pd().linker_area_component().dataspace())
{ }


void Target_copy::checkpoint()
{
	Genode::Lock::Guard guard(_copy_lock);

	// Copy Thread information
	_copy_threads();

	// Copy Capability information
	_copy_capabilities();

	// Copy Region_maps
	_copy_region_maps();
}
