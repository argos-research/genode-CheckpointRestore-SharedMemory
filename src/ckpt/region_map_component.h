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

	Genode::Entrypoint        &_ep;
	Genode::Allocator         &_md_alloc;
	/**
	 * Wrapped region map at core
	 */
	Genode::Region_map_client _parent_rm_client;

	/**
	 * Record of an attached dataspace
	 */
	struct Region : public Genode::List<Region>::Element
	{
		Genode::Dataspace_capability ds_cap;
		Genode::size_t               size;
		Genode::off_t                offset;
		Genode::addr_t               local_addr;

		Region(Genode::Dataspace_capability ds_cap, Genode::size_t size,
				Genode::off_t offset, Genode::addr_t local_addr)
		:
			ds_cap(ds_cap), size(size), offset(offset), local_addr(local_addr)
		{  }

		/**
		 * Find Region which contains the addr
		 */
		Region *find_by_addr(Genode::addr_t addr)
		{
			if((addr >= local_addr) && (addr < local_addr + size))
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
		_parent_rm_client(rm_cap),
		_regions()
	{
		_ep.manage(*this);
		if(verbose) Genode::log("Region_map_component created");
	}

	~Region_map_component()
	{
		_ep.dissolve(*this);
		if(verbose) Genode::log("Region_map_component destroyed");
	}

	/******************************
	 ** Region map Rpc interface **
	 ******************************/

	Local_addr attach(Genode::Dataspace_capability ds_cap, Genode::size_t size, Genode::off_t offset,
			bool use_local_addr, Region_map::Local_addr local_addr, bool executable)
	{
		if (verbose)
			Genode::log("Rm::attach()");

		return _parent_rm_client.attach(ds_cap, size, offset,
				use_local_addr, local_addr, executable);
	}

	void detach(Region_map::Local_addr local_addr)
	{
		if(verbose)
			Genode::log("Rm::detach()");

		// Detach from real region map
		_parent_rm_client.detach(local_addr);
	}

	void fault_handler(Genode::Signal_context_capability handler)
	{
		if(verbose)
			Genode::log("Rm::fault_handler()");

		_parent_rm_client.fault_handler(handler);
	}

	State state()
	{
		if(verbose)
			Genode::log("Rm::state()");

		return _parent_rm_client.state();
	}

	Genode::Dataspace_capability dataspace()
	{
		if(verbose)
			Genode::log("Rm::dataspace()");

		return _parent_rm_client.dataspace();
	}
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
