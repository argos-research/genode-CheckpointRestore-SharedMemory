/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \date   2016-08-12
 */

#include "ram_session.h"

using namespace Rtcr;


Managed_region_map_info *Fault_handler::_find_faulting_mrm_info()
{
	Genode::Region_map::State state;
	Managed_region_map_info *result_info = nullptr;

	Ram_dataspace_info *ramds_info = _ramds_infos.first();
	while(ramds_info && !result_info)
	{
		if(!ramds_info->mrm_info)
			continue;

		Genode::Region_map_client rm_client(ramds_info->mrm_info->region_map_cap);

		if(rm_client.state().type != Genode::Region_map::State::READY)
		{
			result_info = ramds_info->mrm_info;
		}

		ramds_info = ramds_info->next();
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
		Genode::List<Ram_dataspace_info> &ramds_infos)
:
	Thread(env, "managed dataspace pager", 16*1024),
	_receiver(receiver), _ramds_infos(ramds_infos)
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


void Ram_session_component::_destroy_ramds_info(Ram_dataspace_info &ramds_info)
{

	Genode::Ram_dataspace_capability ds_cap =
			Genode::static_cap_cast<Genode::Ram_dataspace>(ramds_info.ds_cap);

	// Remove the Ram_dataspace_info from the list
	_parent_state.ram_dataspaces.remove(&ramds_info);

	// If Ram_dataspace is managed, clean up Managed_region_map_info
	if(ramds_info.mrm_info)
	{
		// Remove pagefault handler from Managed_region_map_info
		_receiver.dissolve(&ramds_info.mrm_info->context);

		// Destroy all Designated_dataspace_infos
		while(Designated_dataspace_info *dd_info = ramds_info.mrm_info->dd_infos.first())
		{
			Genode::Ram_dataspace_capability designated_ds_cap =
					Genode::static_cap_cast<Genode::Ram_dataspace>(dd_info->ds_cap);

			// Remove Designated_dataspace_info from the list
			ramds_info.mrm_info->dd_infos.remove(dd_info);

			// Destroy Designated_dataspace_info
			Genode::destroy(_md_alloc, dd_info);

			// Free from parent
			_parent_ram.free(designated_ds_cap);
		}

		// Destroy Managed_region_map_info
		Genode::destroy(_md_alloc, ramds_info.mrm_info);
	}

	// Destroy Ram_dataspace_info
	Genode::destroy(_md_alloc, &ramds_info);

	// Free from parent
	_parent_ram.free(ds_cap);
}


Ram_session_component::Ram_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::size_t granularity,
		const char *label, const char *creation_args, bool &bootstrap_phase)
:
	_env                (env),
	_md_alloc           (md_alloc),
	_bootstrap_phase    (bootstrap_phase),
	_parent_ram         (env, label),
	_parent_rm          (env),
	_parent_state       (creation_args, bootstrap_phase),
	_receiver           (),
	_page_fault_handler (env, _receiver, _parent_state.ram_dataspaces),
	_granularity        (granularity)
{
	_page_fault_handler.start();

	if(verbose_debug) Genode::log("\033[33m", "Ram", "\033[0m(parent ", _parent_ram, ")");
}


Ram_session_component::~Ram_session_component()
{
	// Destroy all Ram_dataspace_infos
	while(Ram_dataspace_info *rds_info = _parent_state.ram_dataspaces.first())
	{
		_destroy_ramds_info(*rds_info);
	}

	if(verbose_debug) Genode::log("\033[33m", "~Ram", "\033[0m ", _parent_ram);
}


Ram_session_component *Ram_session_component::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Ram_session_component *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


Genode::Ram_dataspace_capability Ram_session_component::alloc(Genode::size_t size, Genode::Cache_attribute cached)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(size=", Genode::Hex(size),")");

	// Use incremental checkpoint
	if(_granularity > 0)
	{
		// Size of a memory page
		Genode::size_t const PAGESIZE = 4096;

		// Size of a designated dataspace which will be associated with a managed dataspace
		Genode::size_t ds_size = PAGESIZE * _granularity;

		// Number of whole dataspaces
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
					Genode::snprintf(args, sizeof(args), "ram_quota=%u", 256*1024);
					_env.parent().upgrade(_parent_rm, args);
				});

		// Create Ram_dataspace_info and a Managed_region_map_info which contains a list of Designated_dataspace_infos
		Genode::Region_map_client new_rm_client(new_region_map_cap);

		Managed_region_map_info *new_mrm_info =
				new (_md_alloc) Managed_region_map_info(new_region_map_cap);

		Ram_dataspace_info *new_ramds_info =
				new (_md_alloc) Ram_dataspace_info(
						Genode::static_cap_cast<Genode::Ram_dataspace>(new_rm_client.dataspace()),
						size, cached, _bootstrap_phase, new_mrm_info);

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
		Genode::Lock::Guard lock_guard(_parent_state.ram_dataspaces_lock);
		_parent_state.ram_dataspaces.insert(new_ramds_info);

		if(verbose_debug)
		{
			Genode::log("  Allocated managed dataspace (",
					"RM=", new_mrm_info->region_map_cap,
					" DS=", new_ramds_info->ds_cap, ")",
					" containing ", num_dataspaces, "*", ds_size,
					" + ", (remaining_dataspace_size == 0 ? "" : "1*"), remaining_dataspace_size, " Dataspaces");
		}

		// Return the stored Ram_dataspace_capability of the Region_map
		return new_ramds_info->ds_cap;
	}
	else
	{

		auto result_cap = _parent_ram.alloc(size, cached);

		// Create a Ram_dataspace_info to monitor the newly created Ram_dataspace
		Ram_dataspace_info *new_rds_info = new (_md_alloc) Ram_dataspace_info(result_cap, size, cached, _bootstrap_phase);
		Genode::Lock::Guard guard(_parent_state.ram_dataspaces_lock);
		_parent_state.ram_dataspaces.insert(new_rds_info);

		if(verbose_debug) Genode::log("  result: ", result_cap);

		return result_cap;
	}
}


void Ram_session_component::free(Genode::Ram_dataspace_capability ds_cap)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(", ds_cap, ")");

	Genode::Lock::Guard lock_guard(_parent_state.ram_dataspaces_lock);

	// Find the Ram_dataspace_info which monitors the given Ram_dataspace
	Ram_dataspace_info *rds_info = _parent_state.ram_dataspaces.first();
	if(rds_info) rds_info = rds_info->find_by_badge(ds_cap.local_name());

	// Ram_dataspace_info found?
	if(rds_info)
	{
		_destroy_ramds_info(*rds_info);
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
	_parent_state.ref_account_cap = ram_session;

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


Ram_session_component *Ram_root::_create_session(const char *args)
{
	if(verbose_debug) Genode::log("Ram_root::\033[33m", __func__, "\033[0m(", args,")");

	// Extracting label from args
	char label_buf[128];
	Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
	label_arg.string(label_buf, sizeof(label_buf), "");

	// Revert ram_quota calculation, because the monitor needs the original session creation argument
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Ram_session_component) + md_alloc()->overhead(sizeof(Ram_session_component));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);

	// Create custom RAM session
	Ram_session_component *new_session =
			new (md_alloc()) Ram_session_component(_env, _md_alloc, _granularity, label_buf, readjusted_args, _bootstrap_phase);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Ram_root::_upgrade_session(Ram_session_component *session, const char *upgrade_args)
{
	if(verbose_debug) Genode::log("Ram_root::\033[33m", __func__, "\033[0m(session ", session->cap(),", args=", upgrade_args,")");

	char ram_quota_buf[32];
	char new_upgrade_args[160];

	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
}


void Ram_root::_destroy_session(Ram_session_component *session)
{
	if(verbose_debug) Genode::log("Ram_root::\033[33m", __func__, "\033[0m(session ", session->cap(),")");

	_session_rpc_objs.remove(session);
	Genode::destroy(_md_alloc, session);
}


Ram_root::Ram_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep,
		Genode::size_t granularity, bool &bootstrap_phase)
:
	Root_component<Ram_session_component>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_granularity      (granularity),
	_objs_lock        (),
	_session_rpc_objs ()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}

Ram_root::~Ram_root()
{
	while(Ram_session_component *obj = _session_rpc_objs.first())
	{
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}

