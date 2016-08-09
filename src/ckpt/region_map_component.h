/*
 * \brief Intercepting Region map
 * \author Denis Huber
 * \date 2016-08-09
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

#include <base/log.h>
#include <base/entrypoint.h>
#include <base/rpc_server.h>
#include <base/allocator.h>
#include <region_map/client.h>
#include <region_map/region_map.h>

namespace Rtcr {
	class Region_map_component;
	using namespace Genode;
}

class Rtcr::Region_map_component : public Rpc_object<Region_map>
{
private:
	static constexpr bool verbose = true;

	Env               &_env;
	Allocator         &_md_alloc;
	Entrypoint        &_ep;
	/**
	 * Wrapped region map at core
	 */
	Region_map_client _parent_region_map;

public:
	/**
	 * Record of an attached dataspace
	 */
	class Region : public List<Region>::Element
	{
	private:
		void                *_start;
		void                *_end;
		off_t                _offset;
		Dataspace_capability _ds_cap;

	public:
		Region(void *start, void *end, off_t offset, Dataspace_capability ds_cap)
		:
			_start(start), _end(end), _offset(offset), _ds_cap(ds_cap)
		{}

		/**
		 * Find Region which contains the addr
		 */
		Region *find_by_addr(void *addr)
		{
			if((addr >= _start) && (addr <= _end))
				return this;
			Region *region = next();
			return region ? region->find_by_addr(addr) : 0;
		}

		void *local_addr() { return _start; }
	};

private:

	/**
	 * Mapping of target's local addresses and dataspaces
	 */
	List<Region> _regions;
	Lock         _region_map_lock;

public:
	/**
	 * Constructor
	 */
	Region_map_component(Env &env, Allocator &md_alloc, Entrypoint &ep,
			Capability<Region_map> parent_region_map)
	:
		_env(env), _md_alloc(md_alloc), _ep(ep),
		_parent_region_map(parent_region_map), _regions()
	{
		_ep.manage(*this);
	}

	~Region_map_component()
	{
		_ep.dissolve(*this);

		// detach all dataspaces
		Region *curr;
		while((curr = _regions.first()))
			detach(curr->local_addr());
	}

	/******************************
	 ** Region map Rpc interface **
	 ******************************/

	Local_addr attach(Dataspace_capability ds_cap, size_t size, off_t offset,
			bool use_local_addr, Region_map::Local_addr local_addr, bool executable)
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
				Region(start_addr, end_addr, offset, ds_cap));

		if(verbose)
			log("Region: ", start_addr, " - ", end_addr);

		return start_addr;
	}

	void detach(Region_map::Local_addr local_addr)
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

	void fault_handler(Signal_context_capability handler)
	{
		if(verbose)
			log(__PRETTY_FUNCTION__);

		_parent_region_map.fault_handler(handler);
	}

	State state()
	{
		if(verbose)
			log(__PRETTY_FUNCTION__);

		return _parent_region_map.state();
	}

	Dataspace_capability dataspace()
	{
		if(verbose)
			log(__PRETTY_FUNCTION__);

		return _parent_region_map.dataspace();
	}
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
