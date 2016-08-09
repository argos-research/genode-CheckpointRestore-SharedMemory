/*
 * \brief Implementation of Region map interface
 * \author Denis Huber
 * \date 2016-08-09
 */

#include "intercepted/region_map_component.h"

/**************************
 ** Region map component **
 **************************/

Rtcr::Region_map::Local_addr
Rtcr::Region_map_component::attach(Dataspace_capability ds_cap,
		size_t size, off_t offset, bool use_local_addr,
		Region_map::Local_addr local_addr, bool executable)
{
	if (verbose)
		log("size = ", size, ", offset = ", offset);

	// Attach dataspace to real region map
	void *start_addr = _parent_region_map.attach(ds_cap, size, offset,
			use_local_addr, local_addr,executable);

	// Store data about the dataspace in form of a Region
	void *end_addr = (void*)((addr_t)start_addr + size - 1);
	Lock::Guard lock_guard(_region_map_lock);
	_regions.insert(new (_md_alloc)
			Region(start_addr, end_addr, ds_cap, offset));

	if(verbose)
		log("Region: ", start_addr, " - ", end_addr);

	return start_addr;
}

void
Rtcr::Region_map_component::detach(Rtcr::Region_map::Local_addr local_addr)
{
	if(verbose)
		log("local_addr = ", (void*)local_addr);

	// Detach from real region map
	_parent_region_map.detach(local_addr);

	// Remove and delete region from region mapping
	Lock::Guard lock_guard(_region_map_lock);
	Region *region = _regions.first()->find_by_addr(local_addr);
	if(!region)
	{
		error("address not in region map");
		return;
	}
	_regions.remove(region);
	destroy(_md_alloc, region);
}

void
Rtcr::Region_map_component::fault_handler(Signal_context_capability handler)
{
	if(verbose)
		log(__PRETTY_FUNCTION__);

	_parent_region_map.fault_handler(handler);
}

Rtcr::Region_map::State
Rtcr::Region_map_component::state()
{
	if(verbose)
		log(__PRETTY_FUNCTION__);

	return _parent_region_map.state();
}

Rtcr::Dataspace_capability Rtcr::Region_map_component::dataspace()
{
	if(verbose)
		log(__PRETTY_FUNCTION__);

	return _parent_region_map.dataspace();
}
