/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \date   2016-08-09
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

/**
 * This custom Region map intercepts the attach and detach methods to monitor and provide the content of this Region map
 */
class Rtcr::Region_map_component : public Genode::Rpc_object<Genode::Region_map>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = false;

	/**
	 * Entrypoint which manages this Region map
	 */
	Genode::Entrypoint        &_ep;
	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator         &_md_alloc;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client _parent_rm;

	/**
	 * Record of an attached dataspace
	 */
	struct Region_info : Genode::List<Region_info>::Element
	{
		Genode::Dataspace_capability ds_cap;
		Genode::size_t               size;
		Genode::off_t                offset;
		Genode::addr_t               local_addr;
		bool                         executable;

		/**
		 * Constructor
		 *
		 * Store necessary information of an attachment to this Region map to be able to recreate the Region map
		 *
		 * \param ds_cap     Capability to the attached dataspace
		 * \param size       Size of the dataspace
		 * \param offset     Offset in the dataspace
		 * \param local_addr Address of the dataspace in parent's region map
		 * \param executable Indicates whether the dataspace is executable
		 */
		Region_info(Genode::Dataspace_capability ds_cap, Genode::size_t size,
				Genode::off_t offset, Genode::addr_t local_addr, bool executable)
		:
			ds_cap(ds_cap), size(size), offset(offset), local_addr(local_addr), executable(executable)
		{  }

		/**
		 * Find Region which contains the addr
		 *
		 * \param  addr Local address in parent's region map
		 * \return Region_info which contains the local address
		 */
		Region_info *find_by_addr(Genode::addr_t addr)
		{
			if((addr >= local_addr) && (addr <= local_addr + size))
				return this;
			Region_info *region = next();
			return region ? region->find_by_addr(addr) : 0;
		}
	};

	/**
	 * List of target's local addresses and their corresponding dataspaces
	 */
	Genode::List<Region_info> _regions;
	/**
	 * Lock to make _regions thread-safe
	 */
	Genode::Lock              _regions_lock;

public:
	/**
	 * Constructor
	 *
	 * \param ep       Entrypoint for managing the custom Region map
	 * \param md_alloc Allocator for attachments
	 * \param rm_cap   Capability to parent's Region map
	 */
	Region_map_component(Genode::Entrypoint &ep, Genode::Allocator &md_alloc, Genode::Capability<Region_map> rm_cap)
	:
		_ep(ep), _md_alloc(md_alloc),
		_parent_rm(rm_cap),
		_regions()
	{
		_ep.manage(*this);
		if(verbose_debug) Genode::log("Region_map_component created");
	}

	/**
	 * Destrcutor
	 */
	~Region_map_component()
	{
		_ep.dissolve(*this);

		// Destroy all Regions through detach method
		Region_info *curr;
		while((curr = _regions.first()))
			detach(curr->local_addr);
		if(verbose_debug) Genode::log("Region_map_component destroyed");
	}

	/******************************
	 ** Region map Rpc interface **
	 ******************************/

	/**
	 * Attaches a dataspace to parent's Region map and stores information about the attachment
	 */
	Local_addr attach(Genode::Dataspace_capability ds_cap, Genode::size_t size, Genode::off_t offset,
			bool use_local_addr, Region_map::Local_addr local_addr, bool executable)
	{
		if(verbose_debug) Genode::log("Rm::attach()");

		// Attach dataspace to real Region map
		Region_map::Local_addr addr = _parent_rm.attach(
				ds_cap, size, offset, use_local_addr, local_addr, executable);

		// Store information about the attachment
		Region_info *region = new (_md_alloc) Region_info(ds_cap, size, offset, addr, executable);

		if(verbose_debug)
		{
			Genode::size_t ds_size = Genode::Dataspace_client(ds_cap).size();
			Genode::size_t num_pages = (Genode::size_t)(ds_size/4096);

			Genode::log("  Attached dataspace ", ds_cap.local_name(),
			" (local) into [", Genode::Hex((Genode::size_t)addr),
			", ", Genode::Hex((Genode::size_t)addr+ds_size), ") ",
			num_pages, num_pages==1?" page":" pages");
		}

		// Store Region_info in a list
		Genode::Lock::Guard lock_guard(_regions_lock);
		_regions.insert(region);

		return addr;
	}

	/**
	 * Detaches the dataspace from parent's region map and destroys the information about the attachment
	 */
	void detach(Region_map::Local_addr local_addr)
	{
		if(verbose_debug) Genode::log("Rm::detach()");

		// Find region
		Genode::Lock::Guard lock_guard(_regions_lock);
		Region_info *region = _regions.first()->find_by_addr((Genode::addr_t)local_addr);
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

		if(verbose_debug) Genode::log("  Detached dataspace for the local address ", (void*)local_addr);
	}

	void fault_handler(Genode::Signal_context_capability handler)
	{
		if(verbose_debug)
			Genode::log("Rm::fault_handler()");

		_parent_rm.fault_handler(handler);
	}

	State state()
	{
		if(verbose_debug)
			Genode::log("Rm::state()");

		return _parent_rm.state();
	}

	Genode::Dataspace_capability dataspace()
	{
		if(verbose_debug)
			Genode::log("Rm::dataspace()");

		return _parent_rm.dataspace();
	}
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
