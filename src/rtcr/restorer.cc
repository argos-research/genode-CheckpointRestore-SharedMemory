/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "restorer.h"

using namespace Rtcr;


Restorer::Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state)
: _alloc(alloc), _child(child), _state(state), _badge_badge_mapping() { }


Restorer::~Restorer()
{

}


void Restorer::restore()
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m()");

	Genode::log("Before: \n", _child);


	//Create RPC objects with checkpointed state
	//_prepare_rm_sessions();
	//_prepare_log_sessions();
	//_prepare_timer_sessions();

	//Recreate state of automatically created RPC objects
	//PD
	//_restore_pd_session(_child.pd(), _state._stored_pd_session);
	//CPU
	//_prepare_cpu_session();
	//  _prepare_threads();
	//RAM
	//_prepare_ram_session();
	//  _prepare_allocated_dataspaces();


	//Adjust capability map/capability array
	//_prepare_capability_map();

	//Insert capabilities into capability space
	//_prepare_capability_space();

	//Genode::List<Badge_info> exclude_infos = _create_exclude_infos(address_space, stack_area, linker_area, rm_root);

	//Copy memory content
	//Genode::List<Badge_dataspace_info> visited_infos;
	//_update_dataspace_content(ram, visited_infos, exclude_infos);
	//_update_dataspace_content(address_space, visited_infos, exclude_infos);
	//_update_dataspace_content(stack_area, visited_infos, exclude_infos);
	//_update_dataspace_content(linker_area, visited_infos, exclude_infos);
	//if(rm_root) _update_dataspace_content(rm_root, visisted_infos, exclude_infos);


	//Delete exclude_infos
	//_destroy_exclude_infos(exclude_infos);
	//Delete visited_infos
	//while(Badge_dataspace_info *info = visited_infos.first())
	//{
	//	visited_infos.remove(info);
	//	Genode::destroy(_alloc, info);
	//}


	Genode::log("After: \n", _child);

}
