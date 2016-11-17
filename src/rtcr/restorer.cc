/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "restorer.h"

using namespace Rtcr;

Restorer::Restorer(Target_child &child, Target_state &state)
: _child(child), _state(state) { }

void Restorer::restore()
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m()");
	// Create RPC objects with checkpointed state
	//_prepare_rm_sessions();
	//_prepare_log_sessions();
	//_prepare_timer_sessions();

	// Recreate state of automatically created RPC objects
	// CPU
	//_prepare_threads();
	// PD
	//_prepare_contexts();
	//_prepare_sources();
	//_prepare_region_map(address_space);
	//_prepare_region_map(stack_area);
	//_prepare_region_map(linker_area);
	//RAM
	//_prepare_ram_dataspaces();


	// Adjust capability map/capability array
	//_prepare_capability_map();

	// Insert capabilities into capability space
	//_prepare_capability_space();

	//Genode::List<Badge_info> exclude_infos = _create_exclude_infos(address_space, stack_area, linker_area, rm_root);

	// Copy memory content
	// Genode::List<Badge_dataspace_info> visited_infos;
	//_update_dataspace_content(ram, visited_infos, exclude_infos);
	//_update_dataspace_content(address_space, visited_infos, exclude_infos);
	//_update_dataspace_content(stack_area, visited_infos, exclude_infos);
	//_update_dataspace_content(linker_area, visited_infos, exclude_infos);
	//if(rm_root) _update_dataspace_content(rm_root, visisted_infos, exclude_infos);


	// Delete exclude_infos
	//_destroy_exclude_infos(exclude_infos);
	// Delete visited_infos
	//while(Badge_dataspace_info *info = visited_infos.first())
	//{
	//	visited_infos.remove(info);
	//	Genode::destroy(_alloc, info);
	//}

}
