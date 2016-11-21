/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \date   2016-08-09
 */

#include "region_map_component.h"

using namespace Rtcr;


Region_map_component::Region_map_component(Genode::Allocator &md_alloc, Genode::Capability<Genode::Region_map> region_map_cap,
		Genode::size_t size, const char *label, bool &bootstrap_phase)
:
	_md_alloc          (md_alloc),
	_bootstrap_phase   (bootstrap_phase),
	_parent_region_map (region_map_cap),
	_parent_state      (size, _parent_region_map.dataspace(), bootstrap_phase),
	_label             (label)
{
	if(verbose_debug) Genode::log("\033[33m", "Rmap", "\033[0m<\033[35m", _label, "\033[0m>(parent ", _parent_region_map, ")");
}


Region_map_component::~Region_map_component()
{
	while(Attached_region_info *obj = _parent_state.normal_objs.first())
	{
		_parent_state.normal_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}

	if(verbose_debug) Genode::log("\033[33m", "~Rmap", "\033[0m<\033[35m", _label,"\033[0m> ", _parent_region_map);
}


Region_map_component *Region_map_component::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Region_map_component *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


Genode::Region_map::Local_addr Region_map_component::attach(Genode::Dataspace_capability ds_cap, Genode::size_t size, Genode::off_t offset,
		bool use_local_addr, Region_map::Local_addr local_addr, bool executable)
{
	if(verbose_debug)
	{
		if(use_local_addr)
		{
			Genode::log("Rmap<\033[35m", _label,"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m(",
					"ds ",       ds_cap,
					", size=",       Genode::Hex(size),
					", offset=",     offset,
					", local_addr=", Genode::Hex(local_addr),
					", exe=",        executable,
					")");
		}
		else
		{
			Genode::log("Rmap<\033[35m", _label,"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m(",
					"ds ",   ds_cap,
					", size=",   Genode::Hex(size),
					", offset=", offset,
					", exe=",    executable,
					")");
		}
	}

	// Attach dataspace to real Region map
	Genode::addr_t addr = _parent_region_map.attach(
			ds_cap, size, offset, use_local_addr, local_addr, executable);

	// Actual size of the attached region; page-aligned
	Genode::size_t actual_size;
	if(size == 0)
	{
		Genode::size_t ds_size = Genode::Dataspace_client(ds_cap).size();
		actual_size = Genode::align_addr(ds_size - offset, 12);
	}
	else
	{
		actual_size = Genode::align_addr(size, 12);
	}
	//Genode::log("  actual_size=", Genode::Hex(actual_size));

	// Store information about the attachment
	Attached_region_info *new_obj = new (_md_alloc) Attached_region_info(ds_cap, actual_size, offset, addr, executable, _bootstrap_phase);

	if(verbose_debug)
	{
		Genode::size_t num_pages = actual_size/4096;

		Genode::log("  Attached dataspace (", ds_cap, ")",
		" into [", Genode::Hex(addr),
		", ", Genode::Hex(addr+actual_size), ") ",
		num_pages, num_pages==1?" page":" pages");
	}

	// Store Attached_region_info in a list
	Genode::Lock::Guard lock_guard(_parent_state.objs_lock);
	_parent_state.normal_objs.insert(new_obj);

	return addr;
}


void Region_map_component::detach(Region_map::Local_addr local_addr)
{
	if(verbose_debug) Genode::log("Rmap<\033[35m", _label,"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m(", "local_addr=", Genode::Hex(local_addr), ")");

	// Detach from real region map
	_parent_region_map.detach(local_addr);

	// Find region
	Genode::Lock::Guard lock_guard(_parent_state.objs_lock);
	Attached_region_info *region = _parent_state.normal_objs.first()->find_by_addr((Genode::addr_t)local_addr);
	if(!region)
	{
		Genode::warning("Region not found in Rm::detach(). Local address ", Genode::Hex(local_addr),
				" not in regions list.");
		return;
	}

	// Remove and destroy region from list and allocator
	_parent_state.normal_objs.remove(region);
	destroy(_md_alloc, region);

	if(verbose_debug) Genode::log("  Detached dataspace from the local address ", Genode::Hex(local_addr));
}


void Region_map_component::fault_handler(Genode::Signal_context_capability handler)
{
	if(verbose_debug)Genode::log("Rmap<\033[35m", _label,"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m(", handler, ")");
	_parent_state.sigh_cap = handler;
	_parent_region_map.fault_handler(handler);
}


Genode::Region_map::State Region_map_component::state()
{
	if(verbose_debug) Genode::log("Rmap<\033[35m", _label,"\033[0m>", "::",
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
	if(verbose_debug) Genode::log("Rmap<\033[35m", _label,"\033[0m>", "::",
			"\033[33m", __func__, "\033[0m()");
	auto result = _parent_region_map.dataspace();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}
