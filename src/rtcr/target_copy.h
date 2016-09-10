/*
 * \brief  Child copy
 * \author Denis Huber
 * \date   2016-09-07
 */

#ifndef _RTCR_TARGET_COPY_H_
#define _RTCR_TARGET_COPY_H_

#include <util/list.h>

namespace Rtcr {
	struct Copied_region_table;
	class Target_copy;
}


struct Rtcr::Copied_region_table : public Genode::List<Rtcr::Copied_region_table>
{
};

class Rtcr::Target_copy : public Genode::List<Rtcr::Target_copy>::Element
{
private:
	Genode::Env                              &_env;
	// Shared resources
	Genode::List<Rtcr::Thread_info>          &_threads;
	Genode::List<Rtcr::Attached_region_info> &_address_space_regions;
	Genode::List<Rtcr::Attached_region_info> &_stack_regions;
	Genode::List<Rtcr::Attached_region_info> &_linker_regions;
	Genode::Allocator                        &_alloc;

	Genode::Lock                             _copy_lock;
	Genode::List<Rtcr::Thread_info>          _copied_threads;
	Genode::List<Rtcr::Attached_region_info> _copied_address_space_regions;
	Genode::List<Rtcr::Attached_region_info> _copied_stack_regions;
	Genode::List<Rtcr::Attached_region_info> _copied_linker_regions;

	Genode::Dataspace_capability _clone_dataspace(Genode::Dataspace_capability &ds_cap)
	{
		Genode::size_t               size      = Genode::Dataspace_client{ds_cap}.size();
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

	void _copy_threads()
	{
		Rtcr::Thread_info *curr_th = _threads.first();
		for( ; curr_th; curr_th = curr_th->next())
		{
			Thread_info *new_th = new (_alloc) Thread_info(curr_th->thread_cap);

			_copied_threads.insert(new_th);
		}
	}

	void _copy_attachments_all()
	{
		Rtcr::Attached_region_info *curr_ar = _stack_regions.first();
		for( ; curr_ar; curr_ar = curr_ar->next())
		{
			// Copy ds


			//Attached_region_info * new_ar = new (_alloc) Attached_region_info();
		}
	}

	void _copy_attachments_inc(Genode::List<Rtcr::Managed_region_info> &managed_regions)
	{
		Rtcr::Attached_region_info *curr_ar = _stack_regions.first();
		for( ; curr_ar; curr_ar = curr_ar->next())
		{
			// Copy ds


			//Attached_region_info * new_ar = new (_alloc) Attached_region_info();
		}
	}

	void _copy_capabilities()
	{
		Genode::log("Implement me!");
	}


public:
	Target_copy(Genode::Env                          &env,
			Genode::List<Rtcr::Thread_info>          &threads,
			Genode::List<Rtcr::Attached_region_info> &address_space_regions,
			Genode::List<Rtcr::Attached_region_info> &stack_regions,
			Genode::List<Rtcr::Attached_region_info> &linker_regions,
			Genode::Allocator                        &alloc)
	:
		_env                         (env),
		_threads                     (threads),
		_address_space_regions       (address_space_regions),
		_stack_regions               (stack_regions),
		_linker_regions              (linker_regions),
		_alloc                       (alloc),
		_copy_lock                   (),
		_copied_threads              (),
		_copied_address_space_regions(),
		_copied_stack_regions        (),
		_copied_linker_regions       ()
	{ }

	void sync(Genode::List<Rtcr::Managed_region_info> *managed_regions)
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

	Genode::List<Rtcr::Thread_info>          &copied_threads()               { return _copied_threads;               }
	Genode::List<Rtcr::Attached_region_info> &copied_address_space_regions() { return _copied_address_space_regions; }
	Genode::List<Rtcr::Attached_region_info> &copied_stack_regions()         { return _copied_stack_regions;         }
	Genode::List<Rtcr::Attached_region_info> &copied_linker_regions()        { return _copied_linker_regions;        }


};

#endif /* _RTCR_TARGET_COPY_H_ */
