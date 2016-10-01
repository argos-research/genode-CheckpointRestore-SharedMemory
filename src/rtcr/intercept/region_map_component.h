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
	struct Attached_region_info;
	class Region_map_component;
}

/**
 * Record of an attached dataspace
 */
struct Rtcr::Attached_region_info : public Genode::List<Attached_region_info>::Element
{
	Genode::Dataspace_capability ds_cap;
	Genode::size_t               size;
	Genode::off_t                offset;
	Genode::addr_t               addr;
	bool                         executable;

	/**
	 * Constructor
	 *
	 * Store necessary information of an attachment to this Region map to be able to recreate the Region map
	 *
	 * \param ds_cap        Capability to the attached dataspace
	 * \param size          Size of the dataspace
	 * \param offset        Offset in the dataspace
	 * \param local_addr    Address of the dataspace in parent's region map
	 * \param executable    Indicates whether the dataspace is executable
	 */
	Attached_region_info(Genode::Dataspace_capability ds_cap, Genode::size_t size,
			Genode::off_t offset, Genode::addr_t local_addr, bool executable)
	:
		ds_cap(ds_cap), size(size), offset(offset), addr(local_addr), executable(executable)
	{  }

	/**
	 * Find Region which contains the addr
	 *
	 * \param  addr Local address in parent's region map
	 * \return Attached_region_info which contains the local address
	 */
	Attached_region_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= this->addr) && (addr <= this->addr + size))
			return this;
		Attached_region_info *region = next();
		return region ? region->find_by_addr(addr) : 0;
	}

	/**
	 * Find Attached_region_info by using a specific Dataspace_capability
	 *
	 * \param cap Dataspace_capability
	 *
	 * \return Attached_region_info with the corresponding Capability
	 */
	Attached_region_info *find_by_cap(Genode::Dataspace_capability cap)
	{
		if(cap == ds_cap)
			return this;
		Attached_region_info *ar_info = next();
		return ar_info ? ar_info->find_by_cap(cap) : 0;
	}
};

/**
 * This custom Region map intercepts the attach and detach methods to monitor and
 * provide the content of this Region map
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
	Genode::Entrypoint                 &_ep;
	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator                  &_md_alloc;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client          _parent_rm;
	/**
	 * Name of the Region map for debugging
	 */
	Genode::String<32>                 _label;
	/**
	 * Lock to make _attached_regions thread-safe
	 */
	Genode::Lock                       _attached_regions_lock;
	/**
	 * List of client's dataspaces and their corresponding local addresses
	 */
	Genode::List<Attached_region_info> _attached_regions;

public:
	/**
	 * Constructor
	 *
	 * \param ep       Entrypoint for managing the custom Region map
	 * \param md_alloc Allocator for attachments
	 * \param rm_cap   Capability to parent's Region map
	 */
	Region_map_component(Genode::Entrypoint &ep, Genode::Allocator &md_alloc, Genode::Capability<Region_map> rm_cap, const char *label)
	:
		_ep(ep),
		_md_alloc(md_alloc),
		_parent_rm(rm_cap),
		_label(label),
		_attached_regions_lock(),
		_attached_regions()
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
		Attached_region_info *curr_at_info;

		Genode::Lock::Guard lock_guard(_attached_regions_lock);

		while((curr_at_info = _attached_regions.first()))
			detach(curr_at_info->addr);
		if(verbose_debug) Genode::log("Region_map_component destroyed");
	}

	/**
	 * Return list of attached regions
	 *
	 * \return Reference to the internal attached regions
	 */
	Genode::List<Attached_region_info> &attached_regions()
	{
		return _attached_regions;
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
		if(verbose_debug)
		{
			if(use_local_addr)
			{
				Genode::log("Rm<", _label.string(),">::\033[33m", "attach", "\033[0m(",
						"ds_cap=",       ds_cap,
						", size=",       Genode::Hex(size, Genode::Hex::PREFIX, Genode::Hex::PAD),
						", offset=",     offset,
						", local_addr=", Genode::Hex(local_addr, Genode::Hex::PREFIX, Genode::Hex::PAD),
						", exe=",        executable?"1":"0",
						")");
			}
			else
			{
				Genode::log("Rm<", _label.string(),">::\033[33m", "attach", "\033[0m(",
						"ds_cap=",   ds_cap,
						", size=",   Genode::Hex(size, Genode::Hex::PREFIX, Genode::Hex::PAD),
						", offset=", offset,
						", exe=",    executable?"1":"0",
						")");
			}
		}

		// Attach dataspace to real Region map
		Region_map::Local_addr addr = _parent_rm.attach(
				ds_cap, size, offset, use_local_addr, local_addr, executable);

		Genode::size_t ds_size = Genode::Dataspace_client{ds_cap}.size();

		// Store information about the attachment
		Attached_region_info *region = new (_md_alloc) Attached_region_info(ds_cap, ds_size, offset, addr, executable);

		if(verbose_debug)
		{
			// Dataspace::size() returns a multiple of 4096 bytes (1 Pagesize)
			Genode::size_t ds_size = Genode::Dataspace_client(ds_cap).size();
			Genode::size_t num_pages = (Genode::size_t)(ds_size/4096);

			Genode::log("  Attached dataspace (", ds_cap, ")",
			" into [", Genode::Hex((Genode::size_t)addr),
			", ", Genode::Hex((Genode::size_t)addr+ds_size), ") ",
			num_pages, num_pages==1?" page":" pages");
		}

		// Store Attached_region_info in a list
		Genode::Lock::Guard lock_guard(_attached_regions_lock);
		_attached_regions.insert(region);

		return addr;
	}

	/**
	 * Detaches the dataspace from parent's region map and destroys the information about the attachment
	 */
	void detach(Region_map::Local_addr local_addr)
	{
		if(verbose_debug)
		{
			Genode::log("Rm<", _label.string(),">::\033[33m", "detach", "\033[0m(",
					"local_addr=", Genode::Hex(local_addr, Genode::Hex::PREFIX, Genode::Hex::PAD),
					")");
		}

		// Detach from real region map
		_parent_rm.detach(local_addr);

		// Find region
		Genode::Lock::Guard lock_guard(_attached_regions_lock);
		Attached_region_info *region = _attached_regions.first()->find_by_addr((Genode::addr_t)local_addr);
		if(!region)
		{
			Genode::warning("Region not found in Rm::detach(). Local address ", Genode::Hex(local_addr),
					" not in regions list.");
			return;
		}

		// Remove and destroy region from list and allocator
		_attached_regions.remove(region);
		destroy(_md_alloc, region);

		if(verbose_debug)
		{
			Genode::log("  Detached dataspace from the local address ", Genode::Hex(local_addr));
		}
	}

	void fault_handler(Genode::Signal_context_capability handler)
	{
		if(verbose_debug)
		{
			Genode::log("Rm<", _label.string(),">::\033[33m", "fault_handler", "\033[0m(",
					"handler_cap=", handler,
					")");
		}

		_parent_rm.fault_handler(handler);
	}

	State state()
	{
		if(verbose_debug)
		{
			Genode::log("Rm<", _label.string(),">::state()");
		}

		return _parent_rm.state();
	}

	Genode::Dataspace_capability dataspace()
	{
		if(verbose_debug)
		{
			Genode::log("Rm<", _label.string(),">::\033[33m", "dataspace", "\033[0m()");
		}

		Genode::Dataspace_capability ds_cap = _parent_rm.dataspace();

		if(verbose_debug)
		{
			Genode::log("  Created managed dataspace (", ds_cap, ")",
					" from Region_map (", _parent_rm, ")");
		}

		return ds_cap;
	}
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
