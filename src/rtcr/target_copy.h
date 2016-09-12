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

/**
 * This list element keeps track whether a dataspace was already cloned
 *
 * Used by copying attachments
 */
struct Rtcr::Copied_dataspace_info : public Genode::List<Copied_dataspace_info>::Element
{
	Genode::Dataspace_capability original;
	Genode::Dataspace_capability clone;

	/**
	 * Given the original dataspace capability, find the corresponding clone dataspace capability
	 *
	 * \param original Dataspace_capability which was used for cloning
	 *
	 * \return Clone dataspace_capability
	 */
	Copied_dataspace_info *find_by_clone_cap(Genode::Dataspace_capability original)
	{
		if(original == this->original)
			return this;
		Copied_dataspace_info *cd_info = next();
		return cd_info ? cd_info->find_by_clone_cap(original) : 0;
	}

	/**
	 * Given the clone dataspace capability, find the corresponding original dataspace capability
	 *
	 * \param clone Dataspace_capability which was created through cloning
	 *
	 * \return Original dataspace_capability
	 */
	Copied_dataspace_info *find_by_original_cap(Genode::Dataspace_capability clone)
	{
		if(clone == this->clone)
			return this;
		Copied_dataspace_info *cd_info = next();
		return cd_info ? cd_info->find_by_original_cap(clone) : 0;
	}
};

/**
 * Struct to manage the copied dataspaces of an original Region_map
 */
struct Rtcr::Copied_region_info : public Genode::List<Copied_region_info>::Element
{
	enum Ds_type { Managed, Foreign };

	Genode::Dataspace_capability ds_cap;
	Genode::addr_t addr;
	Genode::size_t size;
	bool executable;
	Ds_type type;
	Genode::List<Attachable_dataspace_info> *managed_dataspaces;


	Copied_region_info(Genode::Dataspace_capability ds_cap,
			Genode::addr_t addr,
			Genode::size_t size,
			bool executable,
			Ds_type type = Ds_type::Foreign,
			Genode::List<Attachable_dataspace_info> *md = nullptr)
	:
		ds_cap(ds_cap),
		addr(addr),
		size(size),
		executable(executable),
		type(type),
		managed_dataspaces(md)
	{
		if(type == Managed && md == nullptr)
		{
			Genode::error("Copied region is managed but was not provided with a managed dataspaces list");
		}
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
		return (ds_cap == other.ds_cap) &&
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
	 * Clone a dataspace's content
	 *
	 * \param ds_from Dataspace to clone from
	 * \param ds_to   Dataspace to clone to
	 * \param size    Size of cloned memory
	 */
	void _clone_dataspace(Genode::Dataspace_capability &ds_from, Genode::Dataspace_capability &ds_to,
			Genode::size_t size)
	{
		void *orig  = _env.rm().attach(ds_from);
		void *clone = _env.rm().attach(ds_to);

		Genode::memcpy(clone, orig, size);

		_env.rm().detach(clone);
		_env.rm().detach(orig);
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
		Genode::log("copy_attachments_all");
	}

	/**
	 * Copy changed managed-dataspaces and all foreign dataspaces
	 */
	void _copy_attachments_inc(Genode::List<Managed_region_info> &managed_regions)
	{
		Genode::log("copy_attachments_inc");
		//Genode::List<Copied_dataspace_info> copied_dataspaces;

		/*Genode::List

		Attached_region_info *curr_ar = _stack_regions.first();
		for( ; curr_ar; curr_ar = curr_ar->next())
		{
			// Was curr_ar stored in the last checkpoint (i.e. does a corresponding Copied_region_info exists)?
			Copied_region_info *cr_info = _copied_stack_regions.first()->find_corr_region(*curr_ar);

			// A corresponding Copied_region_info exists for curr_ar
			if(cr_info)
			{
				// Is the dataspace of curr_ar managed by Rtcr (i.e. created by Rtcr's custom Ram session)?
				Managed_region_info *mr_info = managed_regions.first()->find_by_cap(curr_ar->ds_cap);

				// curr_ar's dataspace is managed by Rtcr
				if(mr_info && cr_info->type == Copied_region_info::Managed)
				{
					Attachable_dataspace_info *curr_orig_ad   = mr_info->attachable_dataspaces.first();
					Attachable_dataspace_info *curr_copied_ad = cr_info->managed_dataspaces->first();

					for(; curr_orig_ad; curr_orig_ad=curr_orig_ad->next())
					{
						// Dataspace is attached
						if(curr_orig_ad->attached)
						{
							_clone_dataspace(curr_orig_ad->dataspace,
									curr_copied_ad->dataspace, curr_copied_ad->size);
						}
					}

				}
				// The dataspace of curr_ar is not managed
				else
				{

				}

			}
			// curr_ar contains a new region, which was not checkpointed last time
			else
			{

			}



		}*/
	}

	/**
	 * Copy meta information of capabilities
	 */
	void _copy_capabilities()
	{
		Genode::log("Implement me!");
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
