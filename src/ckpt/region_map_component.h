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
}

class Rtcr::Region_map_component : public Genode::Rpc_object<Genode::Region_map>
{
private:
	static constexpr bool verbose = true;

	Genode::Env               &_env;
	Genode::Entrypoint        &_ep;
	Genode::Allocator         &_md_alloc;
	/**
	 * Wrapped region map at core
	 */
	Genode::Region_map_client _parent_region_map;

public:
	/**
	 * Record of an attached dataspace
	 */
	class Region : public Genode::List<Region>::Element
	{
	private:
		void                *_start;
		void                *_end;
		Genode::off_t                _offset;
		Genode::Dataspace_capability _ds_cap;

	public:
		Region(void *start, void *end, Genode::off_t offset, Genode::Dataspace_capability ds_cap)
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
	Genode::List<Region> _regions;
	Genode::Lock         _region_map_lock;

public:
	/**
	 * Constructor
	 */
	Region_map_component(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc,
			Genode::Capability<Region_map> parent_region_map)
	:
		_env(env), _ep(ep), _md_alloc(md_alloc),
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

	Local_addr attach(Genode::Dataspace_capability ds_cap, Genode::size_t size, Genode::off_t offset,
			bool use_local_addr, Region_map::Local_addr local_addr, bool executable)
	{
		if (verbose)
			Genode::log("size = ", size, ", offset = ", offset);

		// Attach dataspace to real region map
		void *start_addr = _parent_region_map.attach(ds_cap, size, offset,
				use_local_addr, local_addr,executable);

		// Store data about the dataspace in form of a Region
		void *end_addr = (void*)((Genode::addr_t)start_addr + size - 1);
		Genode::Lock::Guard lock_guard(_region_map_lock);
		_regions.insert(new (_md_alloc)
				Region(start_addr, end_addr, offset, ds_cap));

		if(verbose)
			Genode::log("Region: ", start_addr, " - ", end_addr);

		return start_addr;
	}

	void detach(Region_map::Local_addr local_addr)
	{
		if(verbose)
			Genode::log("local_addr = ", (void*)local_addr);

		// Detach from real region map
		_parent_region_map.detach(local_addr);

		// Remove and delete region from region mapping
		Genode::Lock::Guard lock_guard(_region_map_lock);
		Region *region = _regions.first()->find_by_addr(local_addr);
		if(!region)
		{
			Genode::error("address not in region map");
			return;
		}
		_regions.remove(region);
		destroy(_md_alloc, region);
	}

	void fault_handler(Genode::Signal_context_capability handler)
	{
		if(verbose)
			Genode::log(__PRETTY_FUNCTION__);

		_parent_region_map.fault_handler(handler);
	}

	State state()
	{
		if(verbose)
			Genode::log(__PRETTY_FUNCTION__);

		return _parent_region_map.state();
	}

	Genode::Dataspace_capability dataspace()
	{
		if(verbose)
			Genode::log(__PRETTY_FUNCTION__);

		return _parent_region_map.dataspace();
	}
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
