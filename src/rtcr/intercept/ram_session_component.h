/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \date   2016-08-12
 */

#ifndef _RTCR_RAM_SESSION_COMPONENT_H_
#define _RTCR_RAM_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <ram_session/connection.h>
#include <rm_session/connection.h>
#include <dataspace/client.h>
#include <util/retry.h>
#include <util/misc_math.h>


namespace Rtcr {
	struct Ram_dataspace_info;
	struct Managed_region_map_info;
	struct Designated_dataspace_info;
	class Fault_handler;
	class Ram_session_component;

	constexpr bool dd_verbose_debug = false;
	constexpr bool fh_verbose_debug = false;
	constexpr bool ram_verbose_debug = false;
}

struct Rtcr::Ram_dataspace_info : Genode::List<Ram_dataspace_info>::Element
{
	/**
	 * Allocated Ram dataspace
	 */
	Genode::Ram_dataspace_capability ram_ds_cap;
	/**
	 * If the pointer is null, then this is an ordinary Ram dataspace.
	 * If the pointer is not null, then this Ram dataspace is managed.
	 * A managed Ram dataspace is a Region map consisting of designated dataspaces.
	 */
	Managed_region_map_info *mrm_info;

	Ram_dataspace_info(Genode::Ram_dataspace_capability ram_ds_cap,
			Managed_region_map_info *mrm_info = nullptr)
	:
		ram_ds_cap(ram_ds_cap),
		mrm_info(mrm_info)
	{}

	Ram_dataspace_info *find_by_cap(Genode::Dataspace_capability cap)
	{
		if(cap == ram_ds_cap)
			return this;
		Ram_dataspace_info *rds_info = next();
		return rds_info ? rds_info->find_by_cap(cap) : 0;
	}
};

/**
 * This struct holds information about a Region map, its designated Ram dataspaces
 */
struct Rtcr::Managed_region_map_info
{
	/**
	 * Region_map which is monitored
	 */
	Genode::Capability<Genode::Region_map>  region_map_cap;
	/**
	 * List of designated Ram dataspaces
	 */
	Genode::List<Designated_dataspace_info> dd_infos;
	/**
	 * Signal context for receiving pagefaults
	 */
	Genode::Signal_context                  context;

	Managed_region_map_info(Genode::Capability<Genode::Region_map> region_map_cap)
	:
		region_map_cap(region_map_cap), dd_infos(), context()
	{ }

};

/**
 * A Designated_dataspace_info belongs to a Managed_region_map_info
 * It holds information about the address in the region map, the size,
 * and whether it is attached in the region map
 */
struct Rtcr::Designated_dataspace_info : public Genode::List<Designated_dataspace_info>::Element
{
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = dd_verbose_debug;

	/**
	 * Reference to the Managed_region_map_info to which this dataspace belongs to
	 */
	Managed_region_map_info      &mrm_info;
	/**
	 * Dataspace which will be attached to / detached from the Managed_region_map_info's Region_map
	 */
	Genode::Dataspace_capability  ds_cap;
	/**
	 * Starting address of the dataspace; it is a relative address, because it is local
	 * to the Region_map to which it will be attached
	 */
	Genode::addr_t                rel_addr;
	/**
	 * Size of the dataspace
	 */
	Genode::size_t                size;
	/**
	 * Indicates whether this dataspace is attached to its Region_map
	 *
	 * If it is set to true, the dataspace ds_cap will be stored during checkpoint,
	 * then it will be detached, and finally this variable will be set to false.
	 */
	bool                          attached;

	/**
	 * Constructor
	 */
	Designated_dataspace_info(Managed_region_map_info &mrm_info, Genode::Dataspace_capability ds_cap,
			Genode::addr_t addr, Genode::size_t size)
	:
		mrm_info(mrm_info), ds_cap(ds_cap), rel_addr(addr), size(size), attached(false)
	{
		// Every new dataspace shall be attached and marked
		attach();
	}

	/**
	 * Find Attachable_dataspace_info which contains the address addr
	 *
	 * \param addr Local address of the corresponding Region_map
	 *
	 * \return Attachable_dataspace_info which contains the local address addr
	 */
	Designated_dataspace_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= rel_addr) && (addr <= rel_addr + size))
			return this;
		Designated_dataspace_info *info = next();
		return info ? info->find_by_addr(addr) : 0;
	}

	/**
	 * Attach dataspace and mark it as attached
	 */
	void attach()
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

	/**
	 * Detach dataspace and mark it as not attached
	 */
	void detach()
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
};

/**
 * \brief Page fault handler designated to handle the page faults caused for the
 * incremental checkpoint mechanism of the custom Ram_session_component
 *
 * Page fault handler which has a list of region maps and their associated dataspaces.
 * Each page fault signal is handled by finding the faulting region map and attaching
 * the designated dataspace to the faulting address.
 */
class Rtcr::Fault_handler : public Genode::Thread
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = fh_verbose_debug;
	/**
	 * Signal_receiver on which the page fault handler waits
	 */
	Genode::Signal_receiver          &_receiver;
	/**
	 * List of region maps and their associated dataspaces
	 * It must contain Managed_region_map_info
	 */
	Genode::List<Ram_dataspace_info> &_rds_infos;

	/**
	 * Find the first faulting Region_map in the list of Ram_dataspaces
	 *
	 * \return Pointer to Managed_region_map_info which contains the faulting Region_map
	 */
	Managed_region_map_info *_find_faulting_mrm_info()
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

	/**
	 * Handle page fault signal by finding the faulting region map and attaching the designated dataspace on the faulting address
	 */
	void _handle_fault()
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

public:
	/**
	 * Constructor
	 */
	Fault_handler(Genode::Env &env, Genode::Signal_receiver &receiver,
			Genode::List<Ram_dataspace_info> &rds_infos)
	:
		Thread(env, "managed dataspace pager", 16*1024),
		_receiver(receiver), _rds_infos(rds_infos)
	{ }

	/**
	 * Entrypoint of the thread
	 * The thread waits for a signal and calls the handler function if it receives any signal
	 */
	void entry()
	{
		while(true)
		{
			Genode::Signal signal = _receiver.wait_for_signal();
			for(unsigned int i = 0; i < signal.num(); ++i)
				_handle_fault();
		}
	}
};

/**
 * Virtual Ram_session to monitor the allocation, freeing, and ram quota transfers
 *
 * Instead of providing ordinary Ram_dataspaces, it can provide managed dataspaces,
 * which are used to monitor the access to the provided Ram_dataspaces
 */
class Rtcr::Ram_session_component : public Genode::Rpc_object<Genode::Ram_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = ram_verbose_debug;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env                       &_env;
	/**
	 * Allocator for structures monitring the allocated Ram_dataspaces
	 */
	Genode::Allocator                 &_md_alloc;
	/**
	 * TODO Needed?
	 */
	Genode::Entrypoint                &_ep;
	/**
	 * Indicator whether to use incremental checkpointing
	 */
	const bool                         _use_inc_ckpt;

	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Genode::Ram_connection             _parent_ram;
	/**
	 * Connection to the parent Rm session for creating new Region_maps (usually core's Rm session)
	 */
	Genode::Rm_connection              _parent_rm;
	/**
	 * Lock to make the list of ram dataspaces thread-safe
	 */
	Genode::Lock                       _rds_infos_lock;
	/**
	 * List of ram dataspaces
	 */
	Genode::List<Ram_dataspace_info>   _rds_infos;
	/**
	 * Receiver of page faults
	 */
	Genode::Signal_receiver            _receiver;
	/**
	 * Fault handler which marks and attaches dataspaces associated with the faulting address
	 */
	Fault_handler                      _page_fault_handler;
	/**
	 * Size of Dataspaces which are associated with the managed dataspace
	 * _granularity is a multiple of a pagesize (4096 Byte)
	 */
	Genode::size_t                     _granularity;


	/**
	 * Destroy rds_info and all its sub infos (unsynchronized)
	 */
	void _destroy_rds_info(Ram_dataspace_info &rds_info)
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

public:

	/**
	 * Constructor
	 */
	Ram_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
			const char *name, bool use_inc_ckpt = true, Genode::size_t granularity = 1)
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

		if(verbose_debug) Genode::log("\033[33m", "Ram_session_component", "\033[0m created");
	}

	/**
	 * Destructor
	 */
	~Ram_session_component()
	{
		// Destroy all Ram_dataspace_infos
		Ram_dataspace_info *rds_info = nullptr;
		while((rds_info = _rds_infos.first()))
		{
			// Implicitly removes the first Ram_dataspace_info from the list,
			// thus, a new "first" list element is assigned to the list
			_destroy_rds_info(*rds_info);
		}

		if(verbose_debug) Genode::log("\033[33m", "Ram_session_component", "\033[0m destructed");
	}

	Genode::Ram_session_capability    parent_cap()          { return _parent_ram.cap(); }
	Genode::List<Ram_dataspace_info> &ram_dataspace_infos() { return _rds_infos;        }

	/***************************
	 ** Ram_session interface **
	 ***************************/

	/**
	 * Allocate a Ram_dataspace
	 *
	 * For managed Ram_dataspaces (_use_inc_ckpt == true), create a new Region_map and allocate
	 * designated Ram_dataspaces for the Region_map and return the Ram_dataspace_capability of the
	 * Region_map.
	 */
	Genode::Ram_dataspace_capability alloc(Genode::size_t size, Genode::Cache_attribute cached) override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "alloc", "\033[0m(size=", Genode::Hex(size),")");

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

	/**
	 * Frees the Ram_dataspace and destroys all monitoring structures
	 */
	void free(Genode::Ram_dataspace_capability ds_cap) override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "free", "\033[0m()");

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

	int ref_account(Genode::Ram_session_capability ram_session) override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "ref_account", "\033[0m(ref=", ram_session, ")");

		auto result = _parent_ram.ref_account(ram_session);

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	// TODO store the Ram quota transfers
	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "transfer_quota", "\033[0m(to=", ram_session, ", size=", amount, ")");

		auto result = _parent_ram.transfer_quota(ram_session, amount);

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	Genode::size_t quota() override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "quota", "\033[0m()");

		auto result = _parent_ram.quota();

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	Genode::size_t used() override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "used", "\033[0m(");

		auto result = _parent_ram.used();

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

};

#endif /* _RTCR_RAM_SESSION_COMPONENT_H_ */
