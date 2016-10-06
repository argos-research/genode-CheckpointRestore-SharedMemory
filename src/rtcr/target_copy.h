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

namespace Rtcr {
	struct Copied_dataspace_info;
	struct Copied_region_deprecated_info;
	struct Copied_region_info;
	class  Target_copy;
}

struct Rtcr::Copied_dataspace_info : public Genode::List<Copied_dataspace_info>::Element
{
	Genode::Dataspace_capability ds_cap;
	/**
	 * Relative address to the original managed dataspace
	 */
	Genode::addr_t rel_addr;
	Genode::size_t size;

	Copied_dataspace_info(Genode::Dataspace_capability ds_cap, Genode::addr_t addr, Genode::size_t size)
	:
		ds_cap   (ds_cap),
		rel_addr (addr),
		size     (size)
	{ }

	/**
	 * Find Copied_dataspace_info which contains the address addr
	 *
	 * \param addr Local address of the corresponding Copied_region_info
	 *
	 * \return Copied_dataspace_info which contains the local address addr
	 */
	Copied_dataspace_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= this->rel_addr) && (addr <= this->rel_addr + size))
			return this;
		Copied_dataspace_info *cd_info = next();
		return cd_info ? cd_info->find_by_addr(addr) : 0;
	}

	/**
	 * Find Copied_dataspace_info which corresponds to a specific Attachable_dataspace_info
	 *
	 * \param ad_info Attachable_dataspace_info
	 *
	 * \return Copied_dataspace_info
	 */
	Copied_dataspace_info *find_by_ad_info(const Attachable_dataspace_info &ad_info)
	{
		if(corresponding_ad_info(ad_info))
			return this;
		Copied_dataspace_info *cd_info = next();
		return cd_info ? cd_info->find_by_ad_info(ad_info) : 0;
	}

	/**
	 * Checks, whether an Attachable_dataspace_info corresponds to a Copied_dataspace_info
	 *
	 * \param ad_info Attachable_dataspace_info
	 *
	 * \return True, if the ad_info corresponds to this Copied_dataspace_info
	 */
	bool corresponding_ad_info(const Attachable_dataspace_info &ad_info)
	{
		return (rel_addr == ad_info.rel_addr) && (size == ad_info.size);
	}

};

/**
 * Struct to manage the copied dataspaces of an original Region_map
 */
struct Rtcr::Copied_region_deprecated_info : public Genode::List<Copied_region_deprecated_info>::Element
{
	enum Ds_type { Managed, Foreign };

	Genode::Dataspace_capability        original_ds_cap;
	Genode::List<Copied_dataspace_info> cloned_dataspaces;
	/**
	 * Address of original dataspace
	 */
	Genode::addr_t                      addr;
	/**
	 * Size of original dataspace
	 */
	Genode::size_t                      size;
	bool                                executable;
	Ds_type                             type;


	Copied_region_deprecated_info(Genode::Dataspace_capability original_ds_cap,
			Genode::addr_t addr,
			Genode::size_t size,
			bool executable,
			Ds_type type)
	:
		original_ds_cap(original_ds_cap),
		cloned_dataspaces(),
		addr(addr),
		size(size),
		executable(executable),
		type(type)
	{ }


	/**
	 * Given the original dataspace capability, find the corresponding clone dataspace capability
	 *
	 * \param original Dataspace_capability which was used for cloning
	 *
	 * \return Clone dataspace_capability
	 */
	Copied_region_deprecated_info *find_by_original_ds_cap(Genode::Dataspace_capability original)
	{
		if(original == original_ds_cap)
			return this;
		Copied_region_deprecated_info *cr_info = next();
		return cr_info ? cr_info->find_by_original_ds_cap(original) : 0;
	}

	/**
	 * Find a corresponding Copied_region_info using an Attached_region_info
	 *
	 * \param ar_info Attached_region_info to look for
	 *
	 * \return Copied_region_info with the corresponding Capability
	 */
	Copied_region_deprecated_info *find_by_ar_info(const Attached_region_info &ar_info)
	{
		if(corresponding(ar_info))
			return this;
		Copied_region_deprecated_info *cr_info = next();
		return cr_info ? cr_info->find_by_ar_info(ar_info) : 0;
	}

	/**
	 * Check if the given Attached_region_info corresponds to this object. A corresponding
	 * region has the same dataspace cap, addr, size and executable flag.
	 *
	 * \param other Other Attached_region_info object to compare to this object
	 *
	 * \return true, if the other region corresponds to this region
	 */
	bool corresponding(const Attached_region_info &other) const
	{
		return (original_ds_cap == other.ds_cap) &&
				(addr == other.addr) &&
				(size == other.size) &&
				(executable == other.executable);
	}

	/**
	 * Counts the list elements from this object to the last object
	 */
	unsigned int distance_to_end()
	{
		Copied_region_deprecated_info *cr_info = next();
		return cr_info ? (cr_info->distance_to_end() + 1)  : 0;
	}
};

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
	Genode::List<Copied_region_deprecated_info>    _copied_address_space_regions;
	Genode::List<Copied_region_deprecated_info>    _copied_stack_regions;
	Genode::List<Copied_region_deprecated_info>    _copied_linker_regions;

	Genode::Dataspace_capability        _stack_ds_cap;
	Genode::Dataspace_capability        _linker_ds_cap;

	/**
	 * Copy dataspace's content
	 *
	 * \param ds_from Dataspace to clone from
	 * \param ds_to   Dataspace to clone to
	 * \param size    Size of cloned memory
	 */
	void _copy_dataspace(Genode::Dataspace_capability &ds_from, Genode::Dataspace_capability &ds_to,
			Genode::size_t size)
	{
		void *orig  = _env.rm().attach(ds_from);
		void *clone = _env.rm().attach(ds_to);

		Genode::memcpy(clone, orig, size);

		_env.rm().detach(clone);
		_env.rm().detach(orig);
	}

	/**
	 * Clone a dataspace including its content
	 *
	 * \param ds_from Dataspace to clone from
	 * \param size    Size of cloned memory
	 *
	 * \return Cloned dataspace cap
	 */
	Genode::Dataspace_capability _clone_dataspace(Genode::Dataspace_capability &ds_from, Genode::size_t size)
	{
		Genode::Dataspace_capability ds_to = _env.ram().alloc(size);

		if(!ds_to.valid())
		{
			Genode::error(__func__, ": Memory allocation for cloned dataspace failed");
			return Genode::Dataspace_capability();
		}

		_copy_dataspace(ds_from, ds_to, size);

		return ds_to;
	}

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
	 * Copy all dataspaces to new dataspaces
	 */
	void _copy_attachments_all()
	{
		_clean_all_entries_in_copied_region_list(_copied_stack_regions);
		_copy_regions_all(_stack_regions, _copied_stack_regions);

		_clean_all_entries_in_copied_region_list(_copied_linker_regions);
		_copy_regions_all(_linker_regions, _copied_stack_regions);

		_clean_all_entries_in_copied_region_list(_copied_address_space_regions);
		_copy_regions_all(_address_space_regions, _copied_address_space_regions);
	}

	void _clean_all_entries_in_copied_region_list(Genode::List<Copied_region_deprecated_info> &copy_list)
	{
		Copied_region_deprecated_info *cr_info = copy_list.first();
		while(cr_info)
		{
			// Store next pointer before it is possibly destroyed
			Copied_region_deprecated_info *next = cr_info->next();

			// Delete all cloned dataspaces
			for(Copied_dataspace_info *cd_info = cr_info->cloned_dataspaces.first(); cd_info; cd_info = cd_info->next())
			{
				cr_info->cloned_dataspaces.remove(cd_info);
				Genode::destroy(_alloc, cd_info);
			}

			// Delete Copied_region_info itself
			copy_list.remove(cr_info);
			Genode::destroy(_alloc, cr_info);

			// Assign pointer to next list element to the current pointer
			cr_info = next;
		}
	}

	/**
	 * Remove all Copied_region_infos with their dataspaces from copy_list, if they are no longer found in orig_list
	 */
	void _clean_old_entries_in_copied_region_list(Genode::List<Attached_region_info> &orig_list, Genode::List<Copied_region_deprecated_info> &copy_list)
	{
		Copied_region_deprecated_info *cr_info = copy_list.first();
		while(cr_info)
		{
			// Store next pointer before it is possibly destroyed
			Copied_region_deprecated_info *next = cr_info->next();

			// Find Attached_region_info which corresponds to Copied_region_info
			Attached_region_info *ar_info = orig_list.first();
			if(ar_info) ar_info = ar_info->find_by_cap(cr_info->original_ds_cap);

			// No corresponding Attached_region_info was found
			if(!ar_info)
			{
				// 1. Free dataspaces and their corresponding cd_info from cr_info
				for(Copied_dataspace_info *cd_info = cr_info->cloned_dataspaces.first(); cd_info; cd_info = cd_info->next())
				{
					_env.ram().free(Genode::static_cap_cast<Genode::Ram_dataspace>(cd_info->ds_cap));
					cr_info->cloned_dataspaces.remove(cd_info);
					Genode::destroy(_alloc, cd_info);
				}

				// 2. Free cr_info
				copy_list.remove(cr_info);
				Genode::destroy(_alloc, cr_info);
			}

			// Assign pointer to next list element to the current pointer
			cr_info = next;
		}
	}


	void _copy_regions_all(Genode::List<Attached_region_info> &orig_list, Genode::List<Copied_region_deprecated_info> &copy_list)
	{
		for(Attached_region_info *ar_info = orig_list.first(); ar_info; ar_info = ar_info->next())
		{
			// Ignore linker and stack area which are disguised as managed dataspaces
			// (e.g. the address space list has the stack area attached as a managed dataspace)
			if(ar_info->ds_cap == _linker_ds_cap || ar_info->ds_cap == _stack_ds_cap)
			{
				continue;
			}

			//Genode::log("Dataspace to copy ", ar_info->ds_cap, " not copied and not managed");

			// 1. Create new Copied_region_info
			Copied_region_deprecated_info *new_cr_info = new (_alloc) Copied_region_deprecated_info(
					ar_info->ds_cap, ar_info->addr, ar_info->size,
					ar_info->executable, Copied_region_deprecated_info::Foreign);

			// 2.2.1 Clone 1 dataspace
			Genode::Dataspace_capability new_ds_cap = _clone_dataspace(ar_info->ds_cap, ar_info->size);

			// 2.2.2 Manage the cloned dataspace with a Copied_dataspace_info
			Copied_dataspace_info *new_cd_info =
					new (_alloc) Copied_dataspace_info(new_ds_cap, 0, ar_info->size);

			// 2.2.3 Insert the new Copied_dataspace_info to the new Copied_region_info
			new_cr_info->cloned_dataspaces.insert(new_cd_info);


			// 3. Attach new Copied_region_info to Copied_region_info list
			copy_list.insert(new_cr_info);

		}
	}

	/**
	 * Copy/Clone all Attached_region_infos from orig, which were changed/are not monitored/are new, to copy
	 */
	void _copy_regions_inc(Genode::List<Attached_region_info> &orig_list, Genode::List<Copied_region_deprecated_info> &copy_list,
			Genode::List<Managed_region_info> &managed_regions, bool do_detach = true)
	{
		// Iterate through original region map (represented by orig_list)
		for(Attached_region_info *ar_info = orig_list.first(); ar_info; ar_info = ar_info->next())
		{
			// Ignore linker and stack area which are disguised as managed dataspaces
			// (e.g. the address space list has the stack area attached as a managed dataspace)
			if(ar_info->ds_cap == _linker_ds_cap || ar_info->ds_cap == _stack_ds_cap)
			{
				continue;
			}

			Managed_region_info *mr_info = nullptr;
			Copied_region_deprecated_info *cr_info = nullptr;

			// Was curr_ar stored in the last checkpoint (i.e. does a corresponding Copied_region_info exists)?
			cr_info = copy_list.first();
			if(cr_info) cr_info = cr_info->find_by_ar_info(*ar_info);

			/*****************************************************************************
			 ** Dataspace from current Attached_region_info was copied in the last sync **
			 *****************************************************************************/
			if(cr_info)
			{
				// Is the dataspace of current Attached_region_info managed by Rtcr (i.e. created by Rtcr's custom Ram session)?
				mr_info = managed_regions.first();
				if(mr_info) mr_info = mr_info->find_by_cap(ar_info->ds_cap);

				/******************************************************************************************
				 ** Dataspace from current Attached_region_info is managed by Rtcr (= managed dataspace) **
				 ******************************************************************************************/
				if(mr_info && cr_info->type == Copied_region_deprecated_info::Managed)
				{
					//Genode::log("Dataspace to copy ", ar_info->ds_cap, " copied and managed");

					// 1. Copy content of n marked dataspaces
					// 2. Unmark dataspaces
					for(Attachable_dataspace_info *ad_info = mr_info->ad_infos.first(); ad_info; ad_info = ad_info->next())
					{
						if(ad_info->attached)
						{
							Copied_dataspace_info *cd_info = cr_info->cloned_dataspaces.first()->find_by_ad_info(*ad_info);

							if(cd_info)
							{
								_copy_dataspace(ad_info->ds_cap, cd_info->ds_cap, ad_info->size);
							}
							else
							{
								Genode::error("No corresponding Copied_dataspace_info found: Attachable_dataspace_info",
										" ds ", ad_info->ds_cap,
										", addr ", ad_info->rel_addr,
										", size ", ad_info->size,
										", managing dataspace ", ad_info->mr_info.mds_cap);
							}

							if(do_detach)
							{
								//Genode::log("  Detaching dataspace ", ad_info->ds_cap
										//," from ", ad_info->ref_managed_region_info.ref_managed_dataspace
								//		);

								ad_info->detach();
							}
						}
					}
				}
				/************************************************************************************
				 ** Dataspace from current Attached_region_info is foreign (= not managed by Rtcr) **
				 ************************************************************************************/
				else
				{
					//Genode::log("Dataspace to copy ", ar_info->ds_cap, " copied and not managed");

					// 1. Copy content of 1 dataspace
					_copy_dataspace(ar_info->ds_cap, cr_info->cloned_dataspaces.first()->ds_cap, ar_info->size);
				}

			}
			/***************************************************************************************
			 ** Dataspace from current Attached_region_info was not copied. It is a new dataspace **
			 ***************************************************************************************/
			else
			{
				//Genode::log("ar_info-ds ", curr_ar->ds_cap, " not in cr_info");

				// Is the dataspace of curr_ar managed by Rtcr (i.e. created by Rtcr's custom Ram session)?
				mr_info = managed_regions.first();
				if(mr_info) mr_info = mr_info->find_by_cap(ar_info->ds_cap);

				/******************************************************************************************
				 ** Dataspace from current Attached_region_info is managed by Rtcr (= managed dataspace) **
				 ******************************************************************************************/
				if(mr_info)
				{
					//Genode::log("Dataspace to copy ", ar_info->ds_cap, " not copied and managed");

					// 1. Create new Copied_region_info
					Copied_region_deprecated_info *new_cr_info = new (_alloc) Copied_region_deprecated_info(
							ar_info->ds_cap, ar_info->addr, ar_info->size,
							ar_info->executable, Copied_region_deprecated_info::Managed);

					// 2. Copy dataspace capabilities/clone dataspaces to cloned_dataspaces from the managed dataspace
					// Test, whether the dataspace was already copied
					cr_info = copy_list.first();
					if(cr_info) cr_info = cr_info->find_by_original_ds_cap(new_cr_info->original_ds_cap);
					if(cr_info)
					{
						// 2.1 Copy dataspace capabilities from already cloned dataspaces
						// Note: Do not detach the cloned dataspaces of this newly created cr_info.
						//       This is done for "copied and managed" dataspaces
						//       or "not copied and managed" dataspaces, which are cloned for the first time.
						new_cr_info->cloned_dataspaces = cr_info->cloned_dataspaces;
					}
					else
					{
						// 2.2.1 Clone n dataspaces
						// 2.2.2 Manage them with n new Copied_dataspace_infos
						// 2.2.3 Insert the new Copied_dataspace_infos to the new Copied_region_info
						// 2.2.4 Detach n dataspaces from Attachable_dataspace_infos to unmark the dataspaces
						Attachable_dataspace_info *ad_info = mr_info->ad_infos.first();
						for(; ad_info; ad_info = ad_info->next())
						{
							Genode::Dataspace_capability new_ds_cap = _clone_dataspace(ad_info->ds_cap, ad_info->size);

							Copied_dataspace_info *new_cd_info =
									new (_alloc) Copied_dataspace_info(new_ds_cap, ad_info->rel_addr, ad_info->size);

							new_cr_info->cloned_dataspaces.insert(new_cd_info);

							if(do_detach)
							{
								//Genode::log("  Detaching dataspace ", ad_info->ds_cap
										//," from ", ad_info->ref_managed_region_info.ref_managed_dataspace
								//		);
								if(ad_info->attached) ad_info->detach();
							}
						}
					}
					// 3. Attach new Copied_region_info to Copied_region_info list
					copy_list.insert(new_cr_info);
				}
				/************************************************************************************
				 ** Dataspace from current Attached_region_info is foreign (= not managed by Rtcr) **
				 ************************************************************************************/
				else
				{
					//Genode::log("Dataspace to copy ", ar_info->ds_cap, " not copied and not managed");

					// 1. Create new Copied_region_info
					Copied_region_deprecated_info *new_cr_info = new (_alloc) Copied_region_deprecated_info(
							ar_info->ds_cap, ar_info->addr, ar_info->size,
							ar_info->executable, Copied_region_deprecated_info::Foreign);

					// 2. Copy/clone 1 dataspace
					// Test, whether the dataspace was already copied
					cr_info = copy_list.first();
					if(cr_info) cr_info = cr_info->find_by_original_ds_cap(new_cr_info->original_ds_cap);
					if(cr_info)
					{
						// 2.1 Copy already cloned dataspaces
						new_cr_info->cloned_dataspaces = cr_info->cloned_dataspaces;
					}
					else
					{
						// 2.2.1 Clone 1 dataspace
						Genode::Dataspace_capability new_ds_cap = _clone_dataspace(ar_info->ds_cap, ar_info->size);

						// 2.2.2 Manage the cloned dataspace with a Copied_dataspace_info
						Copied_dataspace_info *new_cd_info =
								new (_alloc) Copied_dataspace_info(new_ds_cap, 0, ar_info->size);

						// 2.2.3 Insert the new Copied_dataspace_info to the new Copied_region_info
						new_cr_info->cloned_dataspaces.insert(new_cd_info);
					}

					// 3. Attach new Copied_region_info to Copied_region_info list
					copy_list.insert(new_cr_info);
				}
			}
		}
	}

	/**
	 * Copy changed managed-dataspaces and all foreign dataspaces
	 */
	void _copy_attachments_inc(Genode::List<Managed_region_info> &managed_regions)
	{
		_clean_old_entries_in_copied_region_list(_stack_regions, _copied_stack_regions);
		_copy_regions_inc(_stack_regions, _copied_stack_regions, managed_regions, false);

		_clean_old_entries_in_copied_region_list(_linker_regions, _copied_linker_regions);
		_copy_regions_inc(_linker_regions, _copied_linker_regions, managed_regions, false);

		_clean_old_entries_in_copied_region_list(_address_space_regions, _copied_address_space_regions);
		_copy_regions_inc(_address_space_regions, _copied_address_space_regions, managed_regions, false);
	}

	/**
	 * Copy meta information of capabilities
	 */
	void _copy_capabilities()
	{
		Genode::log(__func__, ": Implement me!");
	}

	void _copy_region_maps()
	{
		// Adjust Copy_region_infos of stack area

		// Adjust Copy_region_infos of linker area

		// Adjust Copy_region_infos of address space
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

		// Is the original dataspace a managed dataspace?
		bool managed = false;

		Ram_dataspace_info *rds_info = _ram_dataspace_infos.first();
		if(rds_info) rds_info = rds_info->find_by_cap(orig_info.ds_cap);
		if(rds_info) managed = rds_info->mrm_info != nullptr;

		// Create and insert Copy_region_info
		Copied_region_info *new_copy_info = new (_alloc) Copied_region_info(*orig_info, copy_ds_cap, managed);
		copy_infos.insert(new_copy_info);
	}

	void _copy_dataspaces(Genode::List<Copied_region_info> &copy_infos,
			Genode::List<Attached_region_info> &orig_infos)
	{
		// Iterate through orig_infos
		//   Find corresponding copy_info
		//   Determine whether orig_info's dataspace is managed
		//     Managed: Only copy attached dataspaces
		//     Not_managed: Copy whole attached dataspace
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
		// Copy Thread information
		_copy_threads();

		// Copy Capability information
		_copy_capabilities();

		// Copy dataspaces
		_copy_region_maps();


	}

	Genode::List<Thread_info>        &copied_threads()               { return _copied_threads;               }
	Genode::List<Copied_region_deprecated_info> &copied_address_space_regions() { return _copied_address_space_regions; }
	Genode::List<Copied_region_deprecated_info> &copied_stack_regions()         { return _copied_stack_regions;         }
	Genode::List<Copied_region_deprecated_info> &copied_linker_regions()        { return _copied_linker_regions;        }


};

#endif /* _RTCR_TARGET_COPY_H_ */
