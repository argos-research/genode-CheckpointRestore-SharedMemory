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
#include "intercept/cpu_session_component.h"
#include "intercept/pd_session_component.h"
#include "monitor/copied_region_info.h"

namespace Rtcr {
	class  Target_copy;

	constexpr bool copy_verbose_debug = true;
}

/**
 * Class which holds the information for checkpoint/restore of a Target_child
 */
class Rtcr::Target_copy : public Genode::List<Rtcr::Target_copy>::Element
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = copy_verbose_debug;

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
	void _copy_threads();
	/**
	 * Copy meta information of capabilities
	 */
	void _copy_capabilities();
	/**
	 * Copy the three standard region maps in a component
	 */
	void _copy_region_maps();
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
	void _copy_region_map(Genode::List<Copied_region_info> &copy_infos, Genode::List<Attached_region_info> &orig_infos);
	/**
	 * For each Copied_region_info which has no corresponding Attached_region_info delete the Copied_region_info
	 */
	void _delete_copied_region_infos(Genode::List<Copied_region_info> &copy_infos,
			Genode::List<Attached_region_info> &orig_infos);
	/**
	 * Remove Copied_region_info from its list and free its memory space
	 */
	void _delete_copied_region_info(Copied_region_info &info,
			Genode::List<Copied_region_info> &infos);
	/**
	 * For each Attached_region_info which has no corresponding Copied_region_info create a new Copied_region_info
	 */
	void _create_copied_region_infos(Genode::List<Copied_region_info> &copy_infos,
			Genode::List<Attached_region_info> &orig_infos);
	/**
	 * Creates a corresponding Copy_region_info from an Attached_region_info
	 * and inserts it into copy_infos
	 */
	void _create_copied_region_info(Attached_region_info &orig_info,
			Genode::List<Copied_region_info> &copy_infos);
	/**
	 * \brief Copy the content of dataspaces from orig_infos to the ones of copy_infos
	 *
	 * Copy the content of dataspaces from orig_infos to the ones of copy_infos.
	 * If the original dataspace is managed, then copy only the content of marked dataspaces
	 * to copy's dataspaces
	 */
	void _copy_dataspaces(Genode::List<Copied_region_info> &copy_infos,
			Genode::List<Attached_region_info> &orig_infos);
	/**
	 * Copy only marked dataspaces
	 */
	void _copy_managed_dataspace(Managed_region_map_info &mrm_info, Copied_region_info &copy_info);
	/**
	 * Copy dataspace's content
	 */
	void _copy_dataspace(Genode::Dataspace_capability &source_ds_cap, Genode::Dataspace_capability &dest_ds_cap,
			Genode::size_t size, Genode::off_t dest_offset = 0);


public:
	Target_copy(Genode::Env &env, Genode::Allocator &alloc, Target_child &child);

	Genode::List<Thread_info>        &copied_threads()               { return _copied_threads;               }
	Genode::List<Copied_region_info> &copied_address_space_regions() { return _copied_address_space_regions; }
	Genode::List<Copied_region_info> &copied_stack_regions()         { return _copied_stack_regions;         }
	Genode::List<Copied_region_info> &copied_linker_regions()        { return _copied_linker_regions;        }

	void checkpoint();

};

#endif /* _RTCR_TARGET_COPY_H_ */
