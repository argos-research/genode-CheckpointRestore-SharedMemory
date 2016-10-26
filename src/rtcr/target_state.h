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
#include <base/env.h>

/* Rtcr includes */
#include "child_state/stored_dataspace_info.h"
#include "child_state/stored_region_info.h"
#include "child_state/stored_thread_info.h"

namespace Rtcr {
	class  Target_state;
}

class Rtcr::Target_state
{
private:
	Genode::Env        &_env;
	Genode::Allocator &_alloc;

	Genode::List<Stored_thread_info>    _stored_threads;
	Genode::List<Stored_region_info>    _stored_address_space;
	Genode::List<Stored_region_info>    _stored_stack_area;
	Genode::List<Stored_region_info>    _stored_linker_area;
	Genode::List<Stored_dataspace_info> _stored_dataspaces;

	/**
	 * Deletes all list elements
	 *
	 * It is used primarily by the destructor
	 */
	template <typename T>
	void _delete_list(Genode::List<T> &infos);
	/**
	 * Specialized method to delete Stored_dataspace_infos from list and free the corresponding dataspaces
	 *
	 * It is used primarily by the destructor
	 */
	void _delete_list(Genode::List<Stored_dataspace_info> &infos);
	/**
	 * Copy all members of the list from_infos to to_infos
	 *
	 * It is used primarily by copy constructor
	 */
	template <typename T>
	void _copy_list(Genode::List<T> &from_infos, Genode::List<T> &to_infos);
	/**
	 * Specialized method to copy all list elements of the list from_infos to to_infos and allocate new dataspaces
	 * to store the dataspace content
	 *
	 * It is used primarily by copy constructor
	 */
	void _copy_list(Genode::List<Stored_dataspace_info> &from_infos, Genode::List<Stored_dataspace_info> &to_infos);
	/**
	 * Copy dataspace's content
	 */
	void _copy_dataspace(Genode::Dataspace_capability source_ds_cap, Genode::Dataspace_capability dest_ds_cap,
			Genode::size_t size, Genode::off_t dest_offset = 0);

public:
	Target_state(Genode::Env &env, Genode::Allocator &alloc);
	Target_state(Target_state &other);
	~Target_state();

	Target_state& operator=(const Target_state &other) = delete;
};

#endif /* _RTCR_TARGET_STATE_H_ */
