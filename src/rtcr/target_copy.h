/*
 * \brief  Child copy
 * \author Denis Huber
 * \date   2016-09-07
 */

#ifndef _RTCR_TARGET_COPY_H_
#define _RTCR_TARGET_COPY_H_

#include <util/list.h>

namespace Rtcr {
	struct Copied_region_info;
	class  Target_copy;
}

/**
 * Struct to manage the copied dataspaces of an original Region_map
 */
struct Rtcr::Copied_region_info : public Genode::List<Copied_region_info>::Element
{
	enum Ds_type { Managed, Foreign };

	Genode::Dataspace_capability ds_cap;
	Genode::addr_t addr;
	Genode::size_t size;
	Ds_type type;
	Genode::List<Rtcr::Attachable_dataspace_info> *managed_dataspaces;


	Copied_region_info(Genode::Dataspace_capability ds_cap,
			Genode::addr_t addr,
			Genode::size_t size,
			Ds_type type = Ds_type::Foreign,
			Genode::List<Rtcr::Attachable_dataspace_info> *md = nullptr)
	:
		ds_cap(ds_cap),
		addr(addr),
		size(size),
		type(type),
		managed_dataspaces(md)
	{
		if(type == Ds_type::Managed && md == nullptr)
		{
			Genode::error("Copied region is managed but was not provided with a managed dataspaces list");
		}
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
	 */
	Genode::Dataspace_capability _clone_dataspace(Genode::Dataspace_capability &ds_cap, Genode::size_t size)
	{
		//Genode::size_t size = Genode::Dataspace_client{ds_cap}.size();
		Genode::Dataspace_capability clone_cap = _env.ram().alloc(size);

		if(!clone_cap.valid())
		{
			Genode::error(__func__, ": Memory allocation for cloned dataspace failed");
			return Genode::Dataspace_capability();
		}

		void *orig  = _env.rm().attach(ds_cap);
		void *clone = _env.rm().attach(clone_cap);

		Genode::memcpy(clone, orig, size);

		_env.rm().detach(clone);
		_env.rm().detach(orig);

		return clone_cap;
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
		Attached_region_info *curr_ar = _stack_regions.first();
		for( ; curr_ar; curr_ar = curr_ar->next())
		{
			// Copy ds
		}
	}

	/**
	 * Copy changed managed dataspaces and all foreign dataspaces
	 */
	void _copy_attachments_inc(Genode::List<Rtcr::Managed_region_info> &managed_regions)
	{
		Attached_region_info *curr_ar = _stack_regions.first();
		for( ; curr_ar; curr_ar = curr_ar->next())
		{
			// Copy ds
		}
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
		_address_space_regions       (child.pd().address_space_component()),
		_stack_regions               (child.pd().stack_area_component()),
		_linker_regions              (child.pd().linker_area_component()),
		_copy_lock                   (),
		_copied_threads              (),
		_copied_address_space_regions(),
		_copied_stack_regions        (),
		_copied_linker_regions       ()
	{ }

	void sync(Genode::List<Managed_region_info> *managed_regions)
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

	Genode::List<Thread_info>        &copied_threads()               { return _copied_threads;               }
	Genode::List<Copied_region_info> &copied_address_space_regions() { return _copied_address_space_regions; }
	Genode::List<Copied_region_info> &copied_stack_regions()         { return _copied_stack_regions;         }
	Genode::List<Copied_region_info> &copied_linker_regions()        { return _copied_linker_regions;        }


};

#endif /* _RTCR_TARGET_COPY_H_ */
