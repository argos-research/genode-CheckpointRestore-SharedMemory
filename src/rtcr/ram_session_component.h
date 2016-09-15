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
	struct Managed_region_info;
	struct Attachable_dataspace_info;
	class Fault_handler;
	class Ram_session_component;
}

/**
 * Struct which holds information about a Region_map:
 *
 * - Capability of the Region_map to monitor
 * - Capability of the Region_map's dataspace
 * - List of Dataspaces which shall be attached to the Region_map
 * - Signal_context for handling page faults
 */
struct Rtcr::Managed_region_info : public Genode::List<Managed_region_info>::Element
{
	/**
	 * Region_map which is monitored
	 */
	Genode::Capability<Genode::Region_map>  rm_cap;
	/**
	 * Dataspace capability of rm_cap's corresponding managed dataspace
	 *
	 * It is needed to associate the attached dataspaces in the custom Region_map and
	 * the created managed dataspaces in this Ram_session
	 */
	Genode::Dataspace_capability            mds_cap;
	/**
	 * List of dataspaces which belong to the Region_map
	 */
	Genode::List<Attachable_dataspace_info> ad_infos;
	/**
	 * Signal_context for this Region_map
	 */
	Genode::Signal_context                  context;

	/**
	 * Constructor
	 *
	 * \param region_map Region_map which is monitored
	 * \param mds_cap    Dataspace_capability of region_map's managed dataspace
	 */
	Managed_region_info(Genode::Capability<Genode::Region_map> region_map, Genode::Dataspace_capability mds_cap)
	:
		rm_cap(region_map), mds_cap(mds_cap), ad_infos(), context()
	{ }

	/**
	 * Find Managed_region_info by using a specific Dataspace_capability
	 *
	 * \param cap Dataspace_capability of the corresponding managed dataspace
	 *
	 * \return Managed_region_info with the corresponding Capability
	 */
	Managed_region_info *find_by_cap(Genode::Dataspace_capability cap)
	{
		if(cap == mds_cap)
			return this;
		Managed_region_info *managed_region_info = next();
		return managed_region_info ? managed_region_info->find_by_cap(cap) : 0;
	}
};

/**
 * Struct which holds information about a Dataspace:
 *
 * - Which Region_map does it belong to
 * - where shall it be attached,
 * - how big is it, and
 * - is it attached in the referenced Region_map
 */
struct Rtcr::Attachable_dataspace_info : public Genode::List<Attachable_dataspace_info>::Element
{
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = true;

	/**
	 * Reference Managed_region_info to which this dataspace belongs
	 */
	Rtcr::Managed_region_info    &mr_info;
	/**
	 * Dataspace which will be attached to / detached from the reference Region_map
	 */
	Genode::Dataspace_capability  ds_cap;
	/**
	 * Starting address of the dataspace; it is local to the Region_map to which it will be attached
	 */
	Genode::addr_t                rel_addr;
	/**
	 * Size of the dataspace
	 */
	Genode::size_t                size;
	/**
	 * Indicates whether this dataspace is attached to its Region_map
	 *
	 * If it is set to true, the dataspace will be stored during checkpoint, the dataspace detached,
	 * and this variable will be set to false.
	 */
	bool                          attached;

	/**
	 * Constructor
	 *
	 * \param managed_region_info Reference Managed_region_info which stores the Region_map to which the dataspace belongs
	 * \param ds_cap              Dataspace which will be attached to the Region_map
	 * \param local_addr          Address where the dataspace will be attached at
	 * \param size                Size of the Dataspace
	 * \param attached            Indicates whether the dataspace is attached in the Region_map
	 */
	Attachable_dataspace_info(Rtcr::Managed_region_info &managed_region_info, Genode::Dataspace_capability ds_cap,
			Genode::addr_t local_addr, Genode::size_t size)
	:
		mr_info(managed_region_info), ds_cap(ds_cap), rel_addr(local_addr), size(size), attached(false)
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
	Attachable_dataspace_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= rel_addr) && (addr <= rel_addr + size))
			return this;
		Attachable_dataspace_info *dataspace_info = next();
		return dataspace_info ? dataspace_info->find_by_addr(addr) : 0;
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
				Genode::log("Attaching dataspace ", ds_cap.local_name(),
						" to managed dataspace ", mr_info.mds_cap.local_name(),
						" on location ", Genode::Hex(rel_addr), " (local to Region_map)");
			}

			// Attaching Dataspace to designated location
			Genode::addr_t addr =
					Genode::Region_map_client{mr_info.rm_cap}.attach_at(ds_cap, rel_addr);

			// Dataspace was not attached on the right location
			if(addr != rel_addr)
			{
				Genode::warning("Attachable_dataspace_info::attach Dataspace was not attached on its designated location!");
				Genode::warning("  designated", Genode::Hex(rel_addr), " != attached=", Genode::Hex(addr));
			}

			// Mark as attached
			attached = true;
		}
		else
		{
			Genode::warning("Trying to attach an already attached Dataspace:",
					" DS ", ds_cap.local_name(),
					" MD ", mr_info.mds_cap.local_name(),
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
			Genode::log("Detaching dataspace ", ds_cap.local_name(),
					" from managed dataspace ", mr_info.mds_cap.local_name(),
					" on location ", Genode::Hex(rel_addr), " (local to Region_map)");
		}

		if(attached)
		{
			// Detaching Dataspace
			Genode::Region_map_client{mr_info.rm_cap}.detach(rel_addr);

			// Mark as detached
			attached = false;
		}
		else
		{
			Genode::warning("Trying to detach an already detached Dataspace:",
					" RM ", mr_info.rm_cap.local_name(),
					" MD ", mr_info.mds_cap.local_name(),
					" DS ", ds_cap.local_name(),
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
	static constexpr bool verbose_debug = false;
	/**
	 * Signal_receiver on which the page fault handler waits
	 */
	Genode::Signal_receiver           &_receiver;
	/**
	 * List of region maps and their associated dataspaces
	 */
	Genode::List<Managed_region_info> &_mr_infos;

	/**
	 * Find the first faulting Region_map in the list of managed dataspaces
	 *
	 * \return Pointer to Managed_region_info which contains the faulting Region_map
	 */
	Managed_region_info *_find_faulting_rm_info()
	{
		Genode::Region_map::State state;

		Managed_region_info *curr_md = _mr_infos.first();
		for( ; curr_md; curr_md = curr_md->next())
		{
			Genode::Region_map_client rm_client {curr_md->rm_cap};

			// If the region map is found, cancel the search and do not override the pointer to the corresponding Managed_region_info
			if(rm_client.state().type != Genode::Region_map::State::READY)
			{
				return curr_md;
			}
		}

		return curr_md;
	}

	/**
	 * Handle page fault signal by finding the faulting region map and attaching the designated dataspace on the faulting address
	 */
	void _handle_fault()
	{
		// Find faulting Managed_region_info
		Managed_region_info *faulting_rm_info = _find_faulting_rm_info();

		// Get state of faulting Region_map
		Genode::Region_map::State state = Genode::Region_map_client{faulting_rm_info->rm_cap}.state();

		if(verbose_debug)
		{
		Genode::log("Handle fault: Managed dataspace ",
				faulting_rm_info->mds_cap.local_name(), " state is ",
				state.type == Genode::Region_map::State::READ_FAULT  ? "READ_FAULT"  :
				state.type == Genode::Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
				state.type == Genode::Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
				" pf_addr=", Genode::Hex(state.addr));
		}

		// Find dataspace which contains the faulting address
		Rtcr::Attachable_dataspace_info *ad_info =
				faulting_rm_info->ad_infos.first()->find_by_addr(state.addr);

		// Check if a dataspace was found
		if(!ad_info)
		{
			Genode::warning("No designated dataspace for addr = ", state.addr,
					" in Region_map ", faulting_rm_info->rm_cap.local_name());
			return;
		}

		// Attach found dataspace to its designated address
		ad_info->attach();
	}

public:
	/**
	 * Constructor
	 *
	 * \param env                Environment of creator component (usually rtcr)
	 * \param receiver           Signal_receiver which receives the page fault signals
	 * \param managed_dataspaces Reference to the list of managed dataspaces
	 */
	Fault_handler(Genode::Env &env, Genode::Signal_receiver &receiver,
			Genode::List<Managed_region_info> &managed_dataspaces)
	:
		Thread(env, "managed dataspace pager", 16*1024),
		_receiver(receiver), _mr_infos(managed_dataspaces)
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
 * \brief This custom Ram session intercepts the alloc and free methods to provide an incremental checkpoint mechanism.
 *
 * This custom Ram session intercepts the alloc method to provide the client (e.g. child of rtcr) with managed dataspaces
 * instead of physical dataspaces. Each managed dataspace is a Region map. Each Region map has a number of dataspaces of
 * the same size. Each dataspace has its own location in the Region map. And finally, each managed dataspace has the same
 * page fault handler.
 * After the creation of a managed dataspace all internal dataspaces are detached. When the client accesses a region of the
 * managed dataspace a page fault is caused and caught by the page fault handler. This handler attaches the corresponding
 * dataspace and marks it as attached.
 * Now, rtcr knows which regions were accessed by the child and can copy the accessed dataspaces.
 *
 */
class Rtcr::Ram_session_component : public Genode::Rpc_object<Genode::Ram_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = false;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env                       &_env;
	/**
	 * Allocator for objects belonging to the monitoring of the managed dataspaces (e.g. Managed_region_info and Attachable_dataspace_info)
	 */
	Genode::Allocator                 &_md_alloc;
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
	 * Lock to make the list of managed dataspaces thread-safe
	 */
	Genode::Lock                       _mr_infos_lock;
	/**
	 * List of managed dataspaces (= Region maps) given to the client
	 */
	Genode::List<Managed_region_info>  _mr_infos;
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
	 * Removes page fault handler from Region_map,
	 * deletes the Managed_region_info and its Attachable_dataspace_infos, and
	 * removes the Managed_region_info from managed_dataspaces
	 *
	 * \param rm_info Pointer to the Managed_region_info to delete
	 */
	void _delete_rm_info_and_ad_infos(Managed_region_info &rm_info)
	{
		Attachable_dataspace_info *curr_ds = nullptr;

		// Remove page fault handler from Region_map
		_receiver.dissolve(&rm_info.context);

		// Delete Attachable_dataspace_infos from current Managed_region_info
		while((curr_ds = rm_info.ad_infos.first()))
		{
			// Remove current Attachable_dataspace_info from the list
			rm_info.ad_infos.remove(curr_ds);

			// Delete current Attachable_dataspace_info from the allocator
			Genode::destroy(_md_alloc, curr_ds);
		}

		// Remove current Managed_region_info from the list
		_mr_infos.remove(&rm_info);

		// Delete current Managed_region_info from the allocator
		Genode::destroy(_md_alloc, &rm_info);
	}

public:

	/**
	 * Constructor
	 *
	 * \param env         Environment for creating a session to parent's RAM service
	 * \param md_alloc    Allocator for Managed_region_info and Attachable_dataspace_info
	 * \param name        Label for parent's Ram session
	 * \param granularity Size of Dataspaces designated for the managed dataspaces in multiple of a pagesize
	 */
	Ram_session_component(Genode::Env &env, Genode::Allocator &md_alloc, const char *name,
			bool use_inc_ckpt = true, Genode::size_t granularity = 1)
	:
		_env       (env),
		_md_alloc  (md_alloc),
		_use_inc_ckpt(use_inc_ckpt),
		_parent_ram(env, name),
		_parent_rm (env),
		_mr_infos_lock(),
		_mr_infos(),
		_receiver(),
		_page_fault_handler(env, _receiver, _mr_infos),
		_granularity(granularity)
	{
		_page_fault_handler.start();
		if(verbose_debug) Genode::log("Ram_session_component created");
	}

	/**
	 * Destructor
	 */
	~Ram_session_component()
	{
		Managed_region_info *curr_md = nullptr;

		// Remove page fault handler from all Region_maps,
		// delete all Managed_region_infos with their Attachable_dataspaces_infos
		while((curr_md = _mr_infos.first()))
		{
			// Implicitly removes the first Managed_region_info from the managed dataspaces list
			_delete_rm_info_and_ad_infos(*curr_md);
		}

		if(verbose_debug) Genode::log("Ram_session_component destroyed");
	}

	/**
	 * Return the capability to parent's Ram session
	 */
	Genode::Ram_session_capability parent_cap()
	{
		return _parent_ram.cap();
	}

	/**
	 * Return list of managed regions
	 *
	 * \return Reference to the internal managed regions
	 */
	Genode::List<Managed_region_info> &managed_regions()
	{
		return _mr_infos;
	}

	/***************************
	 ** Ram_session interface **
	 ***************************/

	/**
	 * Allocate a managed dataspace with corresponding dataspaces which are associated with this managed dataspace
	 *
	 * \param size   Size of the managed dataspace
	 * \param cached Not used here; only forwarded
	 *
	 * \return Ram_dataspace_capability of the managed dataspace
	 */
	Genode::Ram_dataspace_capability alloc(Genode::size_t size, Genode::Cache_attribute cached) override
	{
		if(verbose_debug)
		{
			Genode::log("Ram::\033[33m", "alloc", "\033[0m(size=", Genode::Hex(size, Genode::Hex::PREFIX, Genode::Hex::PAD),")");
		}

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
			Genode::Capability<Genode::Region_map> new_region_map =
				Genode::retry<Genode::Rm_session::Out_of_metadata>(
					[&] () { return _parent_rm.create(num_dataspaces*ds_size + remaining_dataspace_size); },
					[&] ()
					{
						char args[Genode::Parent::Session_args::MAX_SIZE];
						Genode::snprintf(args, sizeof(args), "ram_quota=%u", 64*1024);
						_env.parent().upgrade(_parent_rm, args);
					});

			// Create a Region_map_client for the new Region_map for convenience
			Genode::Region_map_client new_rm_client{new_region_map};

			// Create Managed_region_info which contains a list of corresponding dataspaces
			Rtcr::Managed_region_info *new_mr_info = new (_md_alloc)
					Rtcr::Managed_region_info(new_region_map, new_rm_client.dataspace());

			// Set our pagefault handler for the Region_map with its own context
			new_rm_client.fault_handler(_receiver.manage(&(new_mr_info->context)));

			// Allocate num_dataspaces of Dataspaces and associate them with the Region_map
			for(Genode::size_t i = 0; i < num_dataspaces; ++i)
			{
				Genode::Dataspace_capability ds_cap;

				// Try to allocate a dataspace which will be associated with the new region_map
				try
				{
					// eager creation of attachments
					// XXX lazy creation could be an optimization (create dataspaces when they are needed)
					ds_cap = _parent_ram.alloc(ds_size, cached);
				}
				catch(Genode::Ram_session::Quota_exceeded)
				{
					Genode::error("_parent_ram has no memory!");
					return Genode::Capability<Genode::Ram_dataspace>();
				}
				// Prepare arguments for creating a Attachable_dataspace_info
				Genode::addr_t local_addr = ds_size * i;

				// Create an Attachable_dataspace_info
				Attachable_dataspace_info *new_ad_info =
						new (_md_alloc) Attachable_dataspace_info(*new_mr_info, ds_cap, local_addr, ds_size);

				// Insert into Managed_region_infos list of dataspaces
				new_mr_info->ad_infos.insert(new_ad_info);
			}

			// Allocate remaining Dataspace and associate it with the Region_map
			if(remaining_dataspace_size != 0)
			{
				Genode::Dataspace_capability ds_cap;

				// Try to allocate a dataspace which will be associated with the new region_map
				try
				{
					// eager creation of attachments
					// XXX lazy creation could be an optimization (create dataspaces when they are needed)
					ds_cap = _parent_ram.alloc(remaining_dataspace_size, cached);
				}
				catch(Genode::Ram_session::Quota_exceeded)
				{
					Genode::error("_parent_ram has no memory!");
					return Genode::Capability<Genode::Ram_dataspace>();
				}
				// Prepare arguments for creating a Attachable_dataspace_info
				Genode::addr_t local_addr = num_dataspaces * ds_size;

				// Create an Attachable_dataspace_info
				Attachable_dataspace_info *new_ad_info =
						new (_md_alloc) Attachable_dataspace_info(*new_mr_info, ds_cap, local_addr, remaining_dataspace_size);

				// Insert into Managed_region_infos list of dataspaces
				new_mr_info->ad_infos.insert(new_ad_info);
			}

			// Insert new Managed_region_info into _managed_dataspaces
			Genode::Lock::Guard lock_guard(_mr_infos_lock);
			_mr_infos.insert(new_mr_info);

			if(verbose_debug)
			{
				Genode::log("  Allocated managed dataspace (", new_mr_info->mds_cap.local_name(), ")",
						" containing ", num_dataspaces, "*", ds_size,
						" + ", (remaining_dataspace_size == 0 ? "" : "1*"), remaining_dataspace_size, " Dataspaces");
			}

			// Return the stored managed dataspace capability of the Region_map as a Ram_dataspace_capability
			return Genode::static_cap_cast<Genode::Ram_dataspace>(new_mr_info->mds_cap);
		}
		else
		{
			return _parent_ram.alloc(size, cached);
		}
	}

	/**
	 * Frees the managed dataspace and deletes it from _managed_dataspaces
	 *
	 * \param ds Capability of the managed dataspace
	 */
	void free(Genode::Ram_dataspace_capability ds) override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "free", "\033[0m()");

		if(_use_inc_ckpt)
		{
			// Find the Managed_region_info which represents the Region_map with the passed Dataspace_capability
			Managed_region_info *mr_info = _mr_infos.first()->find_by_cap(ds);

			// Check whether a corresponding Managed_region_info exists
			if(!mr_info)
			{
				Genode::warning("Region_map not found in Ram::free(). Capability: ", ds.local_name(),
						" not in managed dataspace list.");
				return;
			}

			// Delete the found Managed_region_info including its Attachable_dataspace_infos
			// and remove it from managed_dataspaces
			Genode::Lock::Guard lock_guard(_mr_infos_lock);
			_delete_rm_info_and_ad_infos(*mr_info);

			_parent_ram.free(ds);
		}
		else
		{
			_parent_ram.free(ds);
		}
	}

	int ref_account(Genode::Ram_session_capability ram_session) override
	{
		if(verbose_debug)
		{
			Genode::log("Ram::\033[33m", "ref_account", "\033[0m(ref=", ram_session.local_name(), ")");
		}

		return _parent_ram.ref_account(ram_session);
	}

	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override
	{
		if(verbose_debug)
		{
			Genode::log("Ram::\033[33m", "transfer_quota", "\033[0m(to=", ram_session.local_name(), ", size=", amount, ")");
		}
		return _parent_ram.transfer_quota(ram_session, amount);
	}

	Genode::size_t quota() override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "quota", "\033[0m()");
		return _parent_ram.quota();
	}

	Genode::size_t used() override
	{
		if(verbose_debug) Genode::log("Ram::\033[33m", "used", "\033[0m(");
		return _parent_ram.used();
	}

};

#endif /* _RTCR_RAM_SESSION_COMPONENT_H_ */
