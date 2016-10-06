/*
 * \brief  Child copy
 * \author Denis Huber
 * \date   2016-09-07
 */

#ifndef _RTCR_TARGET_COPY_H_
#define _RTCR_TARGET_COPY_H_

/* Genode includes */
#include <util/list.h>
#include <region_map/client.h>

/* Rtcr includes */
#include "target_child.h"
#include "intercept/region_map_component.h"
#include "intercept/ram_session_component.h"

namespace Rtcr {
	struct Copied_region_info;
	class  Target_copy;
}

struct Rtcr::Copied_region_info : Genode::List<Copied_region_info>::Element
{
	Genode::Dataspace_capability orig_ds_cap;
	Genode::Dataspace_capability copy_ds_cap;
	Genode::addr_t               rel_addr;
	Genode::size_t               size;
	Genode::off_t                offset;
	bool                         managed;

	Copied_region_info(Genode::Dataspace_capability original,
			Genode::Dataspace_capability copy,
			Genode::addr_t rel_addr, Genode::size_t size, Genode::off_t offset,
			bool managed)
	:
		orig_ds_cap (original),
		copy_ds_cap (copy),
		rel_addr    (rel_addr),
		size        (size),
		offset      (offset),
		managed     (managed)
	{ }

	Copied_region_info(Attached_region_info &orig_info, Genode::Dataspace_capability copy_ds_cap, bool managed)
	:
		orig_ds_cap(orig_info.ds_cap),
		copy_ds_cap(copy_ds_cap),
		rel_addr(orig_info.addr),
		size(orig_info.size),
		offset(orig_info.offset),
		managed(managed)
	{ }

	Copied_region_info *find_by_orig_ds_cap(Genode::Dataspace_capability original)
	{
		if(original == orig_ds_cap)
			return this;
		Copied_region_info *info = next();
		return info ? info->find_by_orig_ds_cap(original) : 0;
	}

	Copied_region_info *find_by_copy_ds_cap(Genode::Dataspace_capability copy)
	{
		if(copy == copy_ds_cap)
			return this;
		Copied_region_info *info = next();
		return info ? info->find_by_copy_ds_cap(copy) : 0;
	}

	Copied_region_info *find_by_ar_info(Attached_region_info &ar_info)
	{
		if(ar_info.ds_cap == copy_ds_cap && ar_info.addr == rel_addr)
			return this;
		Copied_region_info *info = next();
		return info ? info->find_by_ar_info(ar_info) : 0;
	}

};

/**
 * Class which holds the information for checkpoint/restore of a Target_child
 */
class Rtcr::Target_copy : public Genode::List<Rtcr::Target_copy>::Element
{
private:
	Genode::Env                        &_env;
	Genode::Allocator                  &_alloc;
	// Shared resources
	Genode::List<Thread_info>          &_threads;
	Genode::List<Attached_region_info> &_address_space_regions;
	Genode::List<Attached_region_info> &_stack_regions;
	Genode::List<Attached_region_info> &_linker_regions;
	Genode::List<Ram_dataspace_info>   &_ram_dataspace_infos;

	Genode::Lock                        _copy_lock;
	Genode::List<Thread_info>           _copied_threads;
	Genode::List<Copied_region_info>    _copied_address_space_regions;
	Genode::List<Copied_region_info>    _copied_stack_regions;
	Genode::List<Copied_region_info>    _copied_linker_regions;

	Genode::Dataspace_capability        _stack_ds_cap;
	Genode::Dataspace_capability        _linker_ds_cap;

	/**
	 * Copy the capabilities of a thread
	 */
	void _copy_threads()
	{
		Thread_info *curr_th = _threads.first();
		for( ; curr_th; curr_th = curr_th->next())
		{
			Thread_info *new_th = new (_alloc) Thread_info(curr_th->thread_cap);

			_copied_threads.insert(new_th);
		}
	}

	/**
	 * Copy meta information of capabilities
	 */
	void _copy_capabilities()
	{
		Genode::log(__func__, ": Implement me!");
	}

	/**
	 * Copy the three standard region maps in a component
	 */
	void _copy_region_maps()
	{
		// Adjust Copy_region_infos of stack area
		_copy_region_map(_copied_stack_regions, _stack_regions);

		// Adjust Copy_region_infos of linker area
		_copy_region_map(_copied_linker_regions, _linker_regions);

		// Adjust Copy_region_infos of address space
		_copy_region_map(_copied_address_space_regions, _address_space_regions);
	}

	/**
	 * \brief Copy a list of Attached_region_infos to the list of Copied_region_infos
	 *
	 * First, adjust the list of Copied_region_infos to the corresponding list of Attached_region_infos.
	 * Whenever a client detaches a dataspace, the corresponding Attached_region_info is deleted.
	 * If a corresponding Copied_region_info exists, also delete it.
	 * Whenever a client attaches a dataspace, a corresponding Attached_region_info is created.
	 * And a corresponding Copied_region_info has to be created.
	 * Second, copy the content from the dataspaces of the Attached_region_infos to the dataspaces of Copied_region_infos
	 */
	void _copy_region_map(Genode::List<Copied_region_info> &copy_infos, Genode::List<Attached_region_info> &orig_infos)
	{
		// Delete each Copied_region_info, if the corresponding Attached_region_info is gone
		_delete_copied_region_infos(copy_infos, orig_infos);

		// Create each Copied_region_info, if a new Attached_region_info is found
		_create_copied_region_infos(copy_infos, orig_infos);

		// Copy dataspaces
	}

	/**
	 * For each Copied_region_info which has no corresponding Attached_region_info delete the Copied_region_info
	 */
	void _delete_copied_region_infos(Genode::List<Copied_region_info> &copy_infos,
			Genode::List<Attached_region_info> &orig_infos)
	{
		// Iterate through copy_infos and delete all Copied_region_infos which have no corresponding Attached_region_info
		Copied_region_info *copy_info = copy_infos.first();

		while(copy_info)
		{
			Copied_region_info *next = copy_info->next();

			Attached_region_info *orig_info = orig_infos.first();
			if(orig_info) orig_info = orig_info->find_by_cr_info(*copy_info);

			if(!orig_info)
			{
				// Delete Copied_region_info
				_delete_copied_region_info(*copy_info, copy_infos);
			}

			copy_info = next;
		}
	}

	/**
	 * Remove Copied_region_info from its list and free its memory space
	 */
	void _delete_copied_region_info(Copied_region_info &info,
			Genode::List<Copied_region_info> &infos)
	{
		infos.remove(&info);
		Genode::destroy(_alloc, &info);
	}

	/**
	 * For each Attached_region_info which has no corresponding Copied_region_info create a new Copied_region_info
	 */
	void _create_copied_region_infos(Genode::List<Copied_region_info> &copy_infos,
			Genode::List<Attached_region_info> &orig_infos)
	{
		Attached_region_info *orig_info = orig_infos.first();

		while(orig_info)
		{
			// Skip stack and linker area which are attached in address space
			if(orig_info->ds_cap == _stack_ds_cap || orig_info->ds_cap == _linker_ds_cap)
				continue;

			Attached_region_info *next = orig_info->next();

			Copied_region_info *copy_info = copy_infos.first();
			if(copy_info) copy_info = copy_info->find_by_ar_info(*orig_info);

			if(!copy_info)
			{
				// Create a corresponding Copied_region_info
				_create_copied_region_info(*orig_info, copy_infos);
			}

			orig_info = next;
		}
	}

	/**
	 * Creates a corresponding Copy_region_info from an Attached_region_info
	 * and inserts it into copy_infos
	 */
	void _create_copied_region_info(Attached_region_info &orig_info,
			Genode::List<Copied_region_info> &copy_infos)
	{
		// Allocate a dataspace to copy the content of the original dataspace
		Genode::Ram_dataspace_capability copy_ds_cap = _env.ram().alloc(orig_info.size);

		// Determine whether orig_info's dataspace is managed
		bool managed = orig_info.managed_dataspace(_ram_dataspace_infos) != nullptr;

		// Create and insert Copy_region_info
		Copied_region_info *new_copy_info = new (_alloc) Copied_region_info(*orig_info, copy_ds_cap, managed);
		copy_infos.insert(new_copy_info);
	}

	/**
	 * \brief Copy the content of dataspaces from orig_infos to the ones of copy_infos
	 *
	 * Copy the content of dataspaces from orig_infos to the ones of copy_infos.
	 * If the original dataspace is managed, then copy only the content of marked dataspaces
	 * to copy's dataspaces
	 */
	void _copy_dataspaces(Genode::List<Copied_region_info> &copy_infos,
			Genode::List<Attached_region_info> &orig_infos)
	{
		// Iterate through orig_infos
		Attached_region_info *orig_info = orig_infos.first();
		while(orig_info)
		{
			// Find corresponding copy_info
			Copied_region_info *copy_info = copy_infos.first()->find_by_ar_info(*orig_info);
			if(!copy_info) Genode::error("No corresponding Copied_region_info for Attached_region_info ", orig_info->ds_cap);

			// Determine whether orig_info's dataspace is managed
			Managed_region_map_info *mrm_info = orig_info->managed_dataspace(_ram_dataspace_infos);

			if(mrm_info)
			{
				// Managed: Copy only marked dataspaces
				_copy_managed_dataspace(*mrm_info, *copy_info);
			}
			else
			{
				// Not_managed: Copy whole dataspaces
				_copy_dataspace(orig_info->ds_cap, copy_info->copy_ds_cap, orig_info->size);
			}

			orig_info = orig_info->next();
		}
	}

	/**
	 * Copy only marked dataspaces
	 */
	void _copy_managed_dataspace(Managed_region_map_info &mrm_info, Copied_region_info &copy_info)
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

	/**
	 * Copy dataspace's content
	 */
	void _copy_dataspace(Genode::Dataspace_capability &source_ds_cap, Genode::Dataspace_capability &dest_ds_cap,
			Genode::size_t size, Genode::off_t dest_offset = 0)
	{
		char *source = _env.rm().attach(source_ds_cap);
		char *dest   = _env.rm().attach(dest_ds_cap);

		Genode::memcpy(dest + dest_offset, source, size);

		_env.rm().detach(dest);
		_env.rm().detach(source);
	}


public:
	Target_copy(Genode::Env &env, Genode::Allocator &alloc, Target_child &child)
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

	void checkpoint()
	{
		Genode::Lock::Guard guard(_copy_lock);

		// Copy Thread information
		_copy_threads();

		// Copy Capability information
		_copy_capabilities();

		// Copy Region_maps
		_copy_region_maps();


	}

	Genode::List<Thread_info>        &copied_threads()               { return _copied_threads;               }
	Genode::List<Copied_region_info> &copied_address_space_regions() { return _copied_address_space_regions; }
	Genode::List<Copied_region_info> &copied_stack_regions()         { return _copied_stack_regions;         }
	Genode::List<Copied_region_info> &copied_linker_regions()        { return _copied_linker_regions;        }


};

#endif /* _RTCR_TARGET_COPY_H_ */
