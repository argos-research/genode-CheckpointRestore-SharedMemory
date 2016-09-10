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
	class Target_copy;
}

struct Rtcr::Copied_region_info : public Genode::List<Rtcr::Copied_region_info>
{

};

class Rtcr::Target_copy : Genode::List<Rtcr::Target_copy>::Element
{
private:
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

public:
	Target_copy(Genode::List<Rtcr::Thread_info>      &threads,
			Genode::List<Rtcr::Attached_region_info> &address_space_regions,
			Genode::List<Rtcr::Attached_region_info> &stack_regions,
			Genode::List<Rtcr::Attached_region_info> &linker_regions,
			Genode::Allocator                        &alloc)
	:
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

	void copy_all()
	{
		// threads
		Rtcr::Thread_info *curr_th = _threads.first();
		for( ; curr_th; curr_th = curr_th->next())
		{
			Thread_info *new_th = new (_alloc) Thread_info(curr_th->thread_cap);

			_copied_threads.insert(new_th);
		}

		// dataspaces
		Rtcr::Attached_region_info *curr_ar = _stack_regions.first();
		for( ; curr_ar; curr_ar = curr_ar->next())
		{
			//Attached_region_info * new_ar = new (_alloc) Attached_region_info();
		}
	}

	void copy_inc(Genode::List<Rtcr::Managed_region_info> &managed_regions)
	{

	}


};

#endif /* _RTCR_TARGET_COPY_H_ */
