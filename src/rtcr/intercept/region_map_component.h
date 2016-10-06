/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \date   2016-08-09
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <region_map/client.h>
#include <dataspace/client.h>

namespace Rtcr {
	struct Attached_region_info;
	class Region_map_component;

	constexpr bool region_map_verbose_debug = false;
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
	{ }

	/**
	 * If this attached dataspace is managed, return its Managed_region_map_info, else return nullptr
	 */
	/*Managed_region_map_info *managed_dataspace(Genode::List<Ram_dataspace_info> &rds_infos)
	{
		Ram_dataspace_info *rds_info = rds_infos.first();
		if(rds_info) rds_info = rds_info->find_by_cap(ds_cap);
		return rds_info ? rds_info->mrm_info : nullptr;
	}*/

	Attached_region_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= this->addr) && (addr <= this->addr + size))
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_addr(addr) : 0;
	}

	Attached_region_info *find_by_cap(Genode::Dataspace_capability cap)
	{
		if(cap == ds_cap)
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_cap(cap) : 0;
	}

	/*Attached_region_info *find_by_cr_info(Copied_region_info &cr_info)
	{
		if(cr_info.orig_ds_cap == ds_cap && cr_info.rel_addr == addr)
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_cr_info(cr_info) : 0;
	}*/
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
	static constexpr bool verbose_debug = region_map_verbose_debug;

	/**
	 * Entrypoint which manages this object
	 */
	Genode::Entrypoint                 &_ep;
	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator                  &_md_alloc;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client           _parent_region_map;
	/**
	 * Parent's session state
	 */
	struct State_info
	{
		Genode::Signal_context_capability fault_handler {};
	} _parent_state;
	/**
	 * Name of the Region map for debugging
	 */
	Genode::String<32>                  _label;
	/**
	 * Lock to make _attached_regions thread-safe
	 */
	Genode::Lock                        _attached_regions_lock;
	/**
	 * List of client's dataspaces and their corresponding local addresses
	 */
	Genode::List<Attached_region_info>  _attached_regions;

public:
	/**
	 * Constructor
	 */
	Region_map_component(Genode::Entrypoint &ep, Genode::Allocator &md_alloc,
			Genode::Capability<Region_map> rm_cap, const char *label)
	:
		_ep                    (ep),
		_md_alloc              (md_alloc),
		_parent_region_map     (rm_cap),
		_label                 (label),
		_attached_regions_lock (),
		_attached_regions      ()
	{
		_ep.manage(*this);

		if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
	}

	/**
	 * Destrcutor
	 */
	~Region_map_component()
	{
		_ep.dissolve(*this);

		// Destroy all Regions through detach method
		Attached_region_info *curr_at_info = nullptr;

		while((curr_at_info = _attached_regions.first()))
			detach(curr_at_info->addr);

		if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
	}

	Genode::Capability<Genode::Region_map> parent_cap() { return _parent_region_map; }
	Genode::List<Attached_region_info> &attached_regions() { return _attached_regions; }

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
				Genode::log("Rmap<", _label.string(),">::\033[33m", __func__, "\033[0m(",
						"ds ",       ds_cap,
						", size=",       Genode::Hex(size),
						", offset=",     offset,
						", local_addr=", Genode::Hex(local_addr),
						", exe=",        executable?"1":"0",
						")");
			}
			else
			{
				Genode::log("Rmap<", _label.string(),">::\033[33m", __func__, "\033[0m(",
						"ds ",   ds_cap,
						", size=",   Genode::Hex(size),
						", offset=", offset,
						", exe=",    executable?"1":"0",
						")");
			}
		}

		// Attach dataspace to real Region map
		Region_map::Local_addr addr = _parent_region_map.attach(
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
		if(verbose_debug) Genode::log("Rmap<", _label.string(),">::\033[33m", __func__, "\033[0m(", "local_addr=", Genode::Hex(local_addr), ")");

		// Detach from real region map
		_parent_region_map.detach(local_addr);

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

		if(verbose_debug) Genode::log("  Detached dataspace from the local address ", Genode::Hex(local_addr));
	}

	void fault_handler(Genode::Signal_context_capability handler)
	{
		if(verbose_debug)Genode::log("Rmap<", _label.string(),">::\033[33m", __func__, "\033[0m(", handler, ")");
		_parent_state.fault_handler = handler;
		_parent_region_map.fault_handler(handler);
	}

	State state()
	{
		if(verbose_debug) Genode::log("Rmap<", _label.string(),">::\033[33m", __func__, "\033[0m()");

		auto result = _parent_region_map.state();

		const char* type = result.type == Genode::Region_map::State::READ_FAULT ? "READ_FAULT" :
				result.type == Genode::Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
				result.type == Genode::Region_map::State::EXEC_FAULT ? "EXEC_FAULT" : "READY";

		if(verbose_debug) Genode::log("  result: ", type, " pf_addr=", Genode::Hex(result.addr));

		return result;
	}

	Genode::Dataspace_capability dataspace()
	{
		if(verbose_debug) Genode::log("Rmap<", _label.string(),">::\033[33m", __func__, "\033[0m()");

		auto result = _parent_region_map.dataspace();

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
