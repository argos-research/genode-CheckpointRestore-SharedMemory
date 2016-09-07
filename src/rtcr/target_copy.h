/*
 * \brief  Child copy
 * \author Denis Huber
 * \date   2016-09-07
 */

#ifndef _RTCR_TARGET_COPY_H_
#define _RTCR_TARGET_COPY_H_

#include <util/list.h>

namespace Rtcr {
	class Target_copy;
}

class Rtcr::Target_copy : Genode::List<Target_copy>::Element
{
private:
	Genode::List<Rtcr::Thread_info>          &_threads;
	Genode::List<Rtcr::Attached_region_info> &_attached_regions;
	Genode::List<Rtcr::Managed_region_info>  &_managed_regions;
	Genode::Allocator                        &_alloc;

	Genode::Lock                             _copy_lock;
	Genode::List<Rtcr::Thread_info>          _copied_threads;
	Genode::List<Rtcr::Attached_region_info> _copied_regions;

public:
	Target_copy(Genode::List<Rtcr::Thread_info>      &threads,
			Genode::List<Rtcr::Attached_region_info> &attached_regions,
			Genode::List<Rtcr::Managed_region_info>  &managed_regions,
			Genode::Allocator                        &alloc)
	:
		_threads         (threads),
		_attached_regions(attached_regions),
		_managed_regions (managed_regions),
		_alloc           (alloc),
		_copy_lock       (),
		_copied_threads  (),
		_copied_regions  ()
	{ }


};

#endif /* _RTCR_TARGET_COPY_H_ */
