/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \date   2016-08-12
 */

#include "ram_session_component.h"

using namespace Rtcr;


Ram_dataspace_info::Ram_dataspace_info(Genode::Ram_dataspace_capability ram_ds_cap,
		Managed_region_map_info *mrm_info)
:
	ram_ds_cap(ram_ds_cap),
	mrm_info(mrm_info)
{}


Ram_dataspace_info *Ram_dataspace_info::find_by_cap(Genode::Dataspace_capability cap)
{
	if(cap == ram_ds_cap)
		return this;
	Ram_dataspace_info *rds_info = next();
	return rds_info ? rds_info->find_by_cap(cap) : 0;
}


Designated_dataspace_info::Designated_dataspace_info(Managed_region_map_info &mrm_info, Genode::Dataspace_capability ds_cap,
		Genode::addr_t addr, Genode::size_t size)
:
	mrm_info(mrm_info), ds_cap(ds_cap), rel_addr(addr), size(size), attached(false)
{
	// Every new dataspace shall be attached and marked
	attach();
}


Designated_dataspace_info *Designated_dataspace_info::find_by_addr(Genode::addr_t addr)
{
	if((addr >= rel_addr) && (addr <= rel_addr + size))
		return this;
	Designated_dataspace_info *info = next();
	return info ? info->find_by_addr(addr) : 0;
}


void Designated_dataspace_info::attach()
{
	if(!attached)
	{
		if(verbose_debug)
		{
			Genode::log("Attaching dataspace ", ds_cap,
					" to region map ", mrm_info.region_map_cap,
					" on location ", Genode::Hex(rel_addr));
		}

		// Attaching Dataspace to designated location
		Genode::addr_t addr =
				Genode::Region_map_client{mrm_info.region_map_cap}.attach_at(ds_cap, rel_addr);

		// Dataspace was not attached on the right location
		if(addr != rel_addr)
		{
			Genode::warning("Designated_dataspace_info::attach Dataspace was not attached on its designated location!");
			Genode::warning("  designated", Genode::Hex(rel_addr), " != attached=", Genode::Hex(addr));
		}

		// Mark as attached
		attached = true;
	}
	else
	{
		Genode::warning("Designated_dataspace_info::attach Trying to attach an already attached Dataspace:",
				" DS ", ds_cap,
				" RM ", mrm_info.region_map_cap,
				" Loc ", Genode::Hex(rel_addr));
	}
}


void Designated_dataspace_info::detach()
{
	if(verbose_debug)
	{
		Genode::log("Detaching dataspace ", ds_cap,
				" from region map ", mrm_info.region_map_cap,
				" on location ", Genode::Hex(rel_addr), " (local to Region_map)");
	}

	if(attached)
	{
		// Detaching Dataspace
		Genode::Region_map_client{mrm_info.region_map_cap}.detach(rel_addr);

		// Mark as detached
		attached = false;
	}
	else
	{
		Genode::warning("Trying to detach an already detached Dataspace:",
				" DS ", ds_cap,
				" RM ", mrm_info.region_map_cap,
				" Loc ", Genode::Hex(rel_addr));
	}
}


Managed_region_map_info *Fault_handler::_find_faulting_mrm_info()
{
	Genode::Region_map::State state;

	Managed_region_map_info *result_info = nullptr;
	for(Ram_dataspace_info *rds_info = _rds_infos.first();
			rds_info && !result_info; rds_info = rds_info->next())
	{
		if(!rds_info->mrm_info)
			continue;

		Genode::Region_map_client rm_client(rds_info->mrm_info->region_map_cap);

		// If the region map is found, cancel the search and do not override the pointer to the corresponding Managed_region_info
		if(rm_client.state().type != Genode::Region_map::State::READY)
		{
			// The assignment of a non-null value to result_info cancels the for-loop
			result_info = rds_info->mrm_info;
		}
	}

	return result_info;
}


void Fault_handler::_handle_fault()
{
	// Find faulting Managed_region_info
	Managed_region_map_info *faulting_mrm_info = _find_faulting_mrm_info();

	// Get state of faulting Region_map
	Genode::Region_map::State state = Genode::Region_map_client{faulting_mrm_info->region_map_cap}.state();

	if(verbose_debug)
	{
	Genode::log("Handle fault: Region map ",
			faulting_mrm_info->region_map_cap, " state is ",
			state.type == Genode::Region_map::State::READ_FAULT  ? "READ_FAULT"  :
			state.type == Genode::Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
			state.type == Genode::Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
			" pf_addr=", Genode::Hex(state.addr));
	}

	// Find dataspace which contains the faulting address
	Designated_dataspace_info *dd_info = faulting_mrm_info->dd_infos.first();
	if(dd_info) dd_info = dd_info->find_by_addr(state.addr);

	// Check if a dataspace was found
	if(!dd_info)
	{
		Genode::warning("No designated dataspace for addr = ", state.addr,
				" in Region_map ", faulting_mrm_info->region_map_cap);
		return;
	}

	// Attach found dataspace to its designated address
	dd_info->attach();
}


Fault_handler::Fault_handler(Genode::Env &env, Genode::Signal_receiver &receiver,
		Genode::List<Ram_dataspace_info> &rds_infos)
:
	Thread(env, "managed dataspace pager", 16*1024),
	_receiver(receiver), _rds_infos(rds_infos)
{ }


void Fault_handler::entry()
{
	while(true)
	{
		Genode::Signal signal = _receiver.wait_for_signal();
		for(unsigned int i = 0; i < signal.num(); ++i)
			_handle_fault();
	}
}


void Ram_session_component::_destroy_rds_info(Ram_dataspace_info &rds_info)
{
	 // 1. Remove the Ram_dataspace_info from the list
	_rds_infos.remove(&rds_info);

	 // 2. If Ram_dataspace is managed, clean up Managed_region_map_info
	if(rds_info.mrm_info)
	{
		 // 2.1 Remove pagefault handler from Managed_region_map_info
		_receiver.dissolve(&rds_info.mrm_info->context);

		 // 2.2 Destroy all Designated_dataspace_infos
		Designated_dataspace_info *dd_info = nullptr;
		while((dd_info = rds_info.mrm_info->dd_infos.first()))
		{
			// 2.2.1 Remove Designated_dataspace_info from the list
			rds_info.mrm_info->dd_infos.remove(dd_info);

			// 2.2.2 Destroy Designated_dataspace_info
			Genode::destroy(_md_alloc, dd_info);
		}

		 // 2.3 Destroy Managed_region_map_info
		Genode::destroy(_md_alloc, rds_info.mrm_info);
	}

	 // 3. Destroy Ram_dataspace_info
	Genode::destroy(_md_alloc, &rds_info);
}


Ram_session_component::Ram_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
		const char *name, bool use_inc_ckpt, Genode::size_t granularity)
:
	_env                (env),
	_md_alloc           (md_alloc),
	_ep                 (ep),
	_use_inc_ckpt       (use_inc_ckpt),
	_parent_ram         (env, name),
	_parent_rm          (env),
	_rds_infos_lock     (),
	_rds_infos          (),
	_receiver           (),
	_page_fault_handler (env, _receiver, _rds_infos),
	_granularity        (granularity)
{
	_page_fault_handler.start();

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Ram_session_component::~Ram_session_component()
{
	// Destroy all Ram_dataspace_infos
	Ram_dataspace_info *rds_info = nullptr;
	while((rds_info = _rds_infos.first()))
	{
		// Implicitly removes the first Ram_dataspace_info from the list,
		// thus, a new "first" list element is assigned to the list
		_destroy_rds_info(*rds_info);
	}

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Genode::Ram_dataspace_capability Ram_session_component::alloc(Genode::size_t size, Genode::Cache_attribute cached)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(size=", Genode::Hex(size),")");

	if(_use_inc_ckpt)
	{
		// Size of a memory page
		const Genode::size_t PAGESIZE = 4096;

		// Size of a dataspace which will be associated with a managed dataspace
		Genode::size_t ds_size = PAGESIZE * _granularity;

		// Number of whole dataspace
		Genode::size_t num_dataspaces = size / ds_size;

		// Size of the remaining dataspace in a multiple of a pagesize
		Genode::size_t remaining_dataspace_size = Genode::align_addr(size % ds_size, 12); // 12 = log2(4096)

		// Create a Region map; if Rm_session is out of ram_quota, upgrade it
		Genode::Capability<Genode::Region_map> new_region_map_cap =
			Genode::retry<Genode::Rm_session::Out_of_metadata>(
				[&] () { return _parent_rm.create(num_dataspaces*ds_size + remaining_dataspace_size); },
				[&] ()
				{
					char args[Genode::Parent::Session_args::MAX_SIZE];
					Genode::snprintf(args, sizeof(args), "ram_quota=%u", 64*1024);
					_env.parent().upgrade(_parent_rm, args);
				});

		// Create Ram_dataspace_info and a Managed_region_map_info which contains a list of Designated_dataspace_infos
		Genode::Region_map_client new_rm_client(new_region_map_cap);

		Managed_region_map_info *new_mrm_info =
				new (_md_alloc) Managed_region_map_info(new_region_map_cap);

		Ram_dataspace_info *new_rds_info =
				new (_md_alloc) Ram_dataspace_info(
						Genode::static_cap_cast<Genode::Ram_dataspace>(new_rm_client.dataspace()),
						new_mrm_info);

		// Set our pagefault handler for the Region_map with the  context of the Managed_region_map_info
		new_rm_client.fault_handler(_receiver.manage(&new_mrm_info->context));

		// Allocate num_dataspaces of Dataspaces and associate them with the Region_map
		for(Genode::size_t i = 0; i < num_dataspaces; ++i)
		{
			Genode::Dataspace_capability ds_cap;

			// Try to allocate a dataspace which will be associated with the new Region_map
			try
			{
				ds_cap = _parent_ram.alloc(ds_size, cached);
			}
			catch(Genode::Ram_session::Quota_exceeded)
			{
				Genode::error("_parent_ram has no memory!");
				return Genode::Capability<Genode::Ram_dataspace>();
			}

			// Compute designated address for the Region_map
			Genode::addr_t rel_addr = ds_size * i;

			// Create a Designated_dataspace_info
			Designated_dataspace_info *new_dd_info =
					new (_md_alloc) Designated_dataspace_info(*new_mrm_info, ds_cap, rel_addr, ds_size);

			// Insert it into Managed_region_map_info's list
			new_mrm_info->dd_infos.insert(new_dd_info);
		}

		// Allocate remaining Dataspace and associate it with the Region_map
		if(remaining_dataspace_size != 0)
		{
			Genode::Dataspace_capability ds_cap;

			// Try to allocate a dataspace which will be associated with the new region_map
			try
			{
				ds_cap = _parent_ram.alloc(remaining_dataspace_size, cached);
			}
			catch(Genode::Ram_session::Quota_exceeded)
			{
				Genode::error("_parent_ram has no memory!");
				return Genode::Capability<Genode::Ram_dataspace>();
			}

			// Compute designated address for the Region_map (it will be the last address in the Region_map)
			Genode::addr_t local_addr = num_dataspaces * ds_size;

			// Create a Designated_dataspace_info
			Designated_dataspace_info *new_dd_info =
					new (_md_alloc) Designated_dataspace_info(*new_mrm_info, ds_cap, local_addr, remaining_dataspace_size);

			// Insert it into Managed_region_map_info's list
			new_mrm_info->dd_infos.insert(new_dd_info);
		}

		// Insert new Ram_dataspace_info into the list
		Genode::Lock::Guard lock_guard(_rds_infos_lock);
		_rds_infos.insert(new_rds_info);

		if(verbose_debug)
		{
			Genode::log("  Allocated managed dataspace (",
					"RM=", new_mrm_info->region_map_cap,
					" DS=", new_rds_info->ram_ds_cap, ")",
					" containing ", num_dataspaces, "*", ds_size,
					" + ", (remaining_dataspace_size == 0 ? "" : "1*"), remaining_dataspace_size, " Dataspaces");
		}

		// Return the stored Ram_dataspace_capability of the Region_map
		return new_rds_info->ram_ds_cap;
	}
	else
	{

		auto result_cap = _parent_ram.alloc(size, cached);

		// Create a Ram_dataspace_info to monitor the newly created Ram_dataspace
		Ram_dataspace_info *new_rds_info = new (_md_alloc) Ram_dataspace_info(result_cap);
		Genode::Lock::Guard guard(_rds_infos_lock);
		_rds_infos.insert(new_rds_info);

		if(verbose_debug) Genode::log("  result: ", result_cap);

		return result_cap;
	}
}


void Ram_session_component::free(Genode::Ram_dataspace_capability ds_cap)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(", ds_cap, ")");

	Genode::Lock::Guard lock_guard(_rds_infos_lock);

	// Find the Ram_dataspace_info which monitors the given Ram_dataspace
	Ram_dataspace_info *rds_info = _rds_infos.first();
	if(rds_info) rds_info = rds_info->find_by_cap(ds_cap);

	// Ram_dataspace_info found?
	if(rds_info)
	{
		_destroy_rds_info(*rds_info);
		_parent_ram.free(ds_cap);
	}
	else
	{
		Genode::warning(__func__, " Ram_dataspace_info not found for ", ds_cap);
		return;
	}

}

int Ram_session_component::ref_account(Genode::Ram_session_capability ram_session)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(ref=", ram_session, ")");

	auto result = _parent_ram.ref_account(ram_session);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

int Ram_session_component::transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(to=", ram_session, ", size=", amount, ")");

	auto result = _parent_ram.transfer_quota(ram_session, amount);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

Genode::size_t Ram_session_component::quota()
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m()");

	auto result = _parent_ram.quota();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

Genode::size_t Ram_session_component::used()
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m()");

	auto result = _parent_ram.used();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}
