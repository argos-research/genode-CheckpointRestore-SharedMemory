/*
 * \brief  Child copy
 * \author Denis Huber
 * \date   2016-09-07
 */

#ifndef _RTCR_TARGET_COPY_H_
#define _RTCR_TARGET_COPY_H_

#include <util/list.h>
#include <region_map/client.h>
#include "target_child.h"

namespace Rtcr {
	struct Copied_dataspace_info;
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
		ds_cap(ds_cap),
		rel_addr(addr),
		size(size)
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
		Copied_dataspace_info *dataspace_info = next();
		return dataspace_info ? dataspace_info->find_by_addr(addr) : 0;
	}

};

/**
 * Struct to manage the copied dataspaces of an original Region_map
 */
struct Rtcr::Copied_region_info : public Genode::List<Copied_region_info>::Element
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


	Copied_region_info(Genode::Dataspace_capability original_ds_cap,
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
	Copied_region_info *find_by_original_ds_cap(Genode::Dataspace_capability original)
	{
		if(original == original_ds_cap)
			return this;
		Copied_region_info *cr_info = next();
		return cr_info ? cr_info->find_by_original_ds_cap(original) : 0;
	}

	/**
	 * Find a corresponding Copied_region_info using an Attached_region_info
	 *
	 * \param ar_info Attached_region_info to look for
	 *
	 * \return Copied_region_info with the corresponding Capability
	 */
	Copied_region_info *find_by_ar_info(const Attached_region_info &ar_info)
	{
		if(corresponding(ar_info))
			return this;
		Copied_region_info *cr_info = next();
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
				(addr == other.local_addr) &&
				(size == other.size) &&
				(executable == other.executable);
	}

	/**
	 * Counts the list elements from this object to the last object
	 */
	unsigned int distance_to_end()
	{
		Copied_region_info *cr_info = next();
		return cr_info ? (cr_info->distance_to_end() + 1)  : 0;
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

	Genode::Lock                        _copy_lock;
	Genode::List<Thread_info>           _copied_threads;
	Genode::List<Copied_region_info>    _copied_address_space_regions;
	Genode::List<Copied_region_info>    _copied_stack_regions;
	Genode::List<Copied_region_info>    _copied_linker_regions;

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

		void *orig  = _env.rm().attach(ds_from);
		void *clone = _env.rm().attach(ds_to);

		Genode::memcpy(clone, orig, size);

		_env.rm().detach(clone);
		_env.rm().detach(orig);

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
		Genode::log(__func__, ": Implement me");
	}

	/**
	 * Copy changed managed-dataspaces and all foreign dataspaces
	 */
	void _copy_attachments_inc(Genode::List<Managed_region_info> &managed_regions)
	{
		Genode::log(__func__, ": Implementing...");
		Copied_region_info *cr_info = nullptr;
		Managed_region_info *mr_info = nullptr;
		Attached_region_info *ar_info = nullptr;

		// 1. Remove all Copied_region_infos with their dataspaces, if they are no longer attached
		for(cr_info = _copied_stack_regions.first(); cr_info; cr_info = cr_info->next())
		{
			// Find Attached_region_info which corresponds to Copied_region_info
			ar_info = _stack_regions.first();
			if(ar_info) ar_info = ar_info->find_by_cap(cr_info->original_ds_cap);

			// No corresponding Attached_region_info was found
			if(!ar_info)
			{
				// 1. Free dataspaces and their corresponding cd_info from cr_info
				Copied_dataspace_info *cd_info = cr_info->cloned_dataspaces.first();
				for(; cd_info; cd_info = cd_info->next())
				{
					_env.ram().free(Genode::static_cap_cast<Genode::Ram_dataspace>(cd_info->ds_cap));
					cr_info->cloned_dataspaces.remove(cd_info);
					Genode::destroy(_alloc, cd_info);
				}

				// 2. Free cr_info
				_copied_stack_regions.remove(cr_info);
				Genode::destroy(_alloc, cr_info);
			}

		}

		// 2. Copy/Clone all Attached_region_infos which were changed/not monitored/new
		for(ar_info = _stack_regions.first(); ar_info; ar_info = ar_info->next())
		{
			// Was curr_ar stored in the last checkpoint (i.e. does a corresponding Copied_region_info exists)?
			cr_info = _copied_stack_regions.first();
			if(cr_info) cr_info = cr_info->find_by_ar_info(*ar_info);

			/*****************************************************************************
			 ** Dataspace from current Attached_region_info was copied in the last sync **
			 *****************************************************************************/
			if(cr_info)
			{
				// Is the dataspace of curr_ar managed by Rtcr (i.e. created by Rtcr's custom Ram session)?
				mr_info = managed_regions.first();
				if(mr_info) mr_info = mr_info->find_by_cap(ar_info->ds_cap);

				/******************************************************************************************
				 ** Dataspace from current Attached_region_info is managed by Rtcr (= managed dataspace) **
				 ******************************************************************************************/
				if(mr_info && cr_info->type == Copied_region_info::Managed)
				{
					Genode::log("Dataspace to copy ", ar_info->ds_cap.local_name(), " copied and managed");

					// 1. Copy content of n marked dataspaces
					// 2. Unmark dataspaces
				}
				/************************************************************************************
				 ** Dataspace from current Attached_region_info is foreign (= not managed by Rtcr) **
				 ************************************************************************************/
				else
				{
					Genode::log("Dataspace to copy ", ar_info->ds_cap.local_name(), " copied and not managed");

					// 1. Copy content of 1 dataspace
				}

			}
			/***************************************************************************************
			 ** Dataspace from current Attached_region_info was not copied. It is a new dataspace **
			 ***************************************************************************************/
			else
			{
				//Genode::log("ar_info-ds ", curr_ar->ds_cap.local_name(), " not in cr_info");

				// Is the dataspace of curr_ar managed by Rtcr (i.e. created by Rtcr's custom Ram session)?
				mr_info = managed_regions.first();
				if(mr_info) mr_info = mr_info->find_by_cap(ar_info->ds_cap);

				/******************************************************************************************
				 ** Dataspace from current Attached_region_info is managed by Rtcr (= managed dataspace) **
				 ******************************************************************************************/
				if(mr_info)
				{
					Genode::log("Dataspace to copy ", ar_info->ds_cap.local_name(), " not copied and managed");

					// 1. Create new Copied_region_info
					Copied_region_info *new_cr_info = new (_alloc) Copied_region_info(
							ar_info->ds_cap, ar_info->local_addr, ar_info->size,
							ar_info->executable, Copied_region_info::Managed);

					// 2. Clone n dataspaces, manage them with n new Copied_dataspace_infos,
					//    and attach the new Copied_dataspace_infos to the new Copied_region_info
					Attachable_dataspace_info *ad_info = mr_info->attachable_dataspaces.first();
					for(; ad_info; ad_info = ad_info->next())
					{
						Genode::Dataspace_capability new_ds_cap = _clone_dataspace(ad_info->dataspace, ad_info->size);
						Copied_dataspace_info *new_cd_info =
								new (_alloc) Copied_dataspace_info(new_ds_cap, ad_info->local_addr, ad_info->size);
						new_cr_info->cloned_dataspaces.insert(new_cd_info);
					}
					// 3. Attach new Copied_region_info to Copied_region_info list
					_copied_stack_regions.insert(new_cr_info);
				}
				/************************************************************************************
				 ** Dataspace from current Attached_region_info is foreign (= not managed by Rtcr) **
				 ************************************************************************************/
				else
				{
					Genode::log("Dataspace to copy ", ar_info->ds_cap.local_name(), " not copied and not managed");

					// 1. Clone 1 dataspace
					// 2. Create Copied_region_info with 1 Copied_dataspace_info
					// 3. Attach Copied_region_info to Copied_region_info list
				}
			}



		}
	}

	/**
	 * Copy meta information of capabilities
	 */
	void _copy_capabilities()
	{
		Genode::log(__func__, ": Implement me!");
	}


public:
	Target_copy(Genode::Env &env, Genode::Allocator &alloc, Target_child &child)
	:
		_env                         (env),
		_alloc                       (alloc),
		_threads                     (child.cpu().threads()),
		_address_space_regions       (child.pd().address_space_component().attached_regions()),
		_stack_regions               (child.pd().stack_area_component().attached_regions()),
		_linker_regions              (child.pd().linker_area_component().attached_regions()),
		_copy_lock                   (),
		_copied_threads              (),
		_copied_address_space_regions(),
		_copied_stack_regions        (),
		_copied_linker_regions       ()
	{ }

	void sync(Genode::List<Managed_region_info> *managed_regions = nullptr)
	{
		_copy_threads();
		_copy_capabilities();

		if(managed_regions)
		{
			_copy_attachments_inc(*managed_regions);
		}
		else
		{
			_copy_attachments_all();
		}

	}

	Genode::List<Thread_info>          &threads()               { return _threads;               }
	Genode::List<Attached_region_info> &address_space_regions() { return _address_space_regions; }
	Genode::List<Attached_region_info> &stack_regions()         { return _stack_regions;         }
	Genode::List<Attached_region_info> &linker_regions()        { return _linker_regions;        }

	Genode::List<Thread_info>        &copied_threads()               { return _copied_threads;               }
	Genode::List<Copied_region_info> &copied_address_space_regions() { return _copied_address_space_regions; }
	Genode::List<Copied_region_info> &copied_stack_regions()         { return _copied_stack_regions;         }
	Genode::List<Copied_region_info> &copied_linker_regions()        { return _copied_linker_regions;        }


};

#endif /* _RTCR_TARGET_COPY_H_ */
