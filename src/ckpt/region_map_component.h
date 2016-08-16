/*
 * \brief Intercepting Region map
 * \author Denis Huber
 * \date 2016-08-09
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

#include <base/log.h>
#include <base/rpc_server.h>
#include <region_map/client.h>
#include <dataspace/client.h>

namespace Rtcr {
	class Region_map_component;
}

class Rtcr::Region_map_component : public Genode::Rpc_object<Genode::Region_map>
{
private:
	static constexpr bool verbose = false;

	Genode::Entrypoint        &_ep;
	Genode::Allocator         &_md_alloc;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client _parent_rm;

	/**
	 * Record of an attached dataspace
	 */
	struct Region : public Genode::List<Region>::Element
	{
		Genode::Dataspace_capability ds_cap;
		Genode::size_t               size;
		Genode::off_t                offset;
		Genode::addr_t               local_addr;
		bool                         executable;

		Region(Genode::Dataspace_capability ds_cap, Genode::size_t size,
				Genode::off_t offset, Genode::addr_t local_addr, bool executable)
		:
			ds_cap(ds_cap), size(size), offset(offset), local_addr(local_addr), executable(executable)
		{  }

		/**
		 * Find Region which contains the addr
		 */
		Region *find_by_addr(Genode::addr_t addr)
		{
			if((addr >= local_addr) && (addr <= local_addr + size))
				return this;
			Region *region = next();
			return region ? region->find_by_addr(addr) : 0;
		}
	};

	/**
	 * List of target's local addresses and their corresponding dataspaces
	 */
	Genode::List<Region> _regions;
	Genode::Lock         _region_map_lock;

public:
	/**
	 * Constructor
	 */
	Region_map_component(Genode::Entrypoint &ep, Genode::Allocator &md_alloc, Genode::Capability<Region_map> rm_cap)
	:
		_ep(ep), _md_alloc(md_alloc),
		_parent_rm(rm_cap),
		_regions()
	{
		_ep.manage(*this);
		if(verbose) Genode::log("Region_map_component created");
	}

	~Region_map_component()
	{
		_ep.dissolve(*this);

		// Destroy all Regions through detach method
		Region *curr;
		while((curr = _regions.first()))
			detach(curr->local_addr);
		if(verbose) Genode::log("Region_map_component destroyed");
	}

	/******************************
	 ** Region map Rpc interface **
	 ******************************/

	Local_addr attach(Genode::Dataspace_capability ds_cap, Genode::size_t size, Genode::off_t offset,
			bool use_local_addr, Region_map::Local_addr local_addr, bool executable)
	{
		if (verbose) Genode::log("Rm::attach()");

		// Attach dataspace to real region map
		Region_map::Local_addr addr = _parent_rm.attach(
				ds_cap, size, offset, use_local_addr, local_addr, executable);

		// Store information about the attachment in a Region
		Region *region = new (_md_alloc) Region(ds_cap, size, offset, addr, executable);

		if(verbose)
		{
			Genode::size_t ds_size = Genode::Dataspace_client(ds_cap).size();
			Genode::size_t size_in_pages = (Genode::size_t)(ds_size/4096);

			Genode::log("  Attached dataspace ", ds_cap.local_name(),
			" (local) to local address ", (void*)addr,
			" with size = ", ds_size,
			" (", size_in_pages,
			size_in_pages==1?" page)":" pages)");
		}

		// Store Region in a list
		Genode::Lock::Guard lock_guard(_region_map_lock);
		_regions.insert(region);

		return addr;
	}

	void detach(Region_map::Local_addr local_addr)
	{
		if(verbose) Genode::log("Rm::detach()");

		// Find region
		Genode::Lock::Guard lock_guard(_region_map_lock);
		Region *region = _regions.first()->find_by_addr((Genode::addr_t)local_addr);
		if(!region)
		{
			Genode::error("Detaching failed. Local address ", (void*)local_addr,
					" not in regions list.");
			return;
		}

		// Remove and destroy region from list and allocator
		_regions.remove(region);
		destroy(_md_alloc, region);

		// Detach from real region map
		_parent_rm.detach(local_addr);

		if(verbose) Genode::log("  Detached dataspace for the local address ", (void*)local_addr);
	}

	void fault_handler(Genode::Signal_context_capability handler)
	{
		if(verbose)
			Genode::log("Rm::fault_handler()");

		_parent_rm.fault_handler(handler);
	}

	State state()
	{
		if(verbose)
			Genode::log("Rm::state()");

		return _parent_rm.state();
	}

	Genode::Dataspace_capability dataspace()
	{
		if(verbose)
			Genode::log("Rm::dataspace()");

		return _parent_rm.dataspace();
	}
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
