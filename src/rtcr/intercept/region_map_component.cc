/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \date   2016-08-09
 */

#include "region_map_component.h"

using namespace Rtcr;


Region_map_component::Region_map_component(Genode::Entrypoint &ep, Genode::Allocator &md_alloc,
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

	if(verbose_debug) Genode::log("\033[33m", "Rmap", "\033[0m<\033[35m", _label.string(),"\033[0m>(parent ", _parent_region_map, ")");
}


Region_map_component::~Region_map_component()
{
	_ep.dissolve(*this);

	// Destroy all Regions through detach method
	Attached_region_info *curr_at_info = nullptr;

	while((curr_at_info = _attached_regions.first()))
		detach(curr_at_info->rel_addr);

	if(verbose_debug) Genode::log("\033[33m", "~Rmap", "\033[0m<\033[35m", _label.string(),"\033[0m> ", _parent_region_map);
}


Genode::Region_map::Local_addr Region_map_component::attach(Genode::Dataspace_capability ds_cap, Genode::size_t size, Genode::off_t offset,
		bool use_local_addr, Region_map::Local_addr local_addr, bool executable)
{
	if(verbose_debug)
	{
		if(use_local_addr)
		{
			Genode::log("Rmap<\033[35m", _label.string(),"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m(",
					"ds ",       ds_cap,
					", size=",       Genode::Hex(size),
					", offset=",     offset,
					", local_addr=", Genode::Hex(local_addr),
					", exe=",        executable?"1":"0",
					")");
		}
		else
		{
			Genode::log("Rmap<\033[35m", _label.string(),"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m(",
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


void Region_map_component::detach(Region_map::Local_addr local_addr)
{
	if(verbose_debug) Genode::log("Rmap<\033[35m", _label.string(),"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m(", "local_addr=", Genode::Hex(local_addr), ")");

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


void Region_map_component::fault_handler(Genode::Signal_context_capability handler)
{
	if(verbose_debug)Genode::log("Rmap<\033[35m", _label.string(),"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m(", handler, ")");
	_parent_state.fault_handler = handler;
	_parent_region_map.fault_handler(handler);
}


Genode::Region_map::State Region_map_component::state()
{
	if(verbose_debug) Genode::log("Rmap<\033[35m", _label.string(),"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m()");

	auto result = _parent_region_map.state();

	const char* type = result.type == Genode::Region_map::State::READ_FAULT ? "READ_FAULT" :
			result.type == Genode::Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
			result.type == Genode::Region_map::State::EXEC_FAULT ? "EXEC_FAULT" : "READY";

	if(verbose_debug) Genode::log("  result: ", type, " pf_addr=", Genode::Hex(result.addr));

	return result;
}


Genode::Dataspace_capability Region_map_component::dataspace()
{
	if(verbose_debug) Genode::log("Rmap<\033[35m", _label.string(),"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m()");

	auto result = _parent_region_map.dataspace();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}
