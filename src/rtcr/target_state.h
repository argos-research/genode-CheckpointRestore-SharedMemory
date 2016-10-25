/*
 * \brief  Target's state
 * \author Denis Huber
 * \date   2016-10-25
 */

#ifndef _RTCR_TARGET_STATE_H_
#define _RTCR_TARGET_STATE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>

/* Rtcr includes */
#include "child_state/stored_region_info.h"
#include "child_state/stored_thread_info.h"
#include "child_state/stored_ram_ds_info.h"

namespace Rtcr {
	class  Target_state;
}

class Rtcr::Target_state
{
private:
	Genode::Allocator &_alloc;

	Genode::List<Stored_thread_info> _stored_threads;
	Genode::List<Stored_region_info> _stored_address_space;
	Genode::List<Stored_region_info> _stored_stack_area;
	Genode::List<Stored_region_info> _stored_linker_area;

	/**
	 * Deletes all members of the list
	 *
	 * It is used primarily by the destructor
	 */
	template <typename T>
	void _delete_list(Genode::List<T> &infos);
	/**
	 * Specialized method to delete Stored_region_infos from list and free the corresponding RAM dataspaces
	 *
	 * It is used primarily by the destructor
	 */
	void _delete_list(Genode::List<Stored_region_info> &infos);
	/**
	 * Copy all members of the list from_infos to to_infos
	 *
	 * It is used primarily by copy constructor
	 */
	template <typename T>
	void _copy_list(Genode::List<T> &from_infos, Genode::List<T> &to_infos);

public:
	Target_state(Genode::Allocator &alloc);
	Target_state(Target_state &other);
	~Target_state();
};

#endif /* _RTCR_TARGET_STATE_H_ */
