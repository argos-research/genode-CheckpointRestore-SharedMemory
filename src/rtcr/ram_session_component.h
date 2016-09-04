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


namespace Rtcr {
	struct Region_map_info;
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
struct Rtcr::Region_map_info : public Genode::List<Region_map_info>::Element
{
	/**
	 * Region_map which is monitored
	 */
	Genode::Capability<Genode::Region_map>        ref_region_map;
	/**
	 * Dataspace capability of ref_region_map's corresponding managed dataspace
	 *
	 * It is needed to associate the attached dataspaces in the custom Region_map and
	 * the created managed dataspaces in this Ram_session
	 */
	Genode::Dataspace_capability                  ref_managed_dataspace;
	/**
	 * List of dataspaces which belong to the Region_map
	 */
	Genode::List<Rtcr::Attachable_dataspace_info> attachable_dataspaces;
	/**
	 * Signal_context for this Region_map
	 */
	Genode::Signal_context                        context;

	/**
	 * Constructor
	 *
	 * \param region_map Region_map which is monitored
	 * \param md_cap     Dataspace_capability of region_map's corresponding managed dataspace
	 */
	Region_map_info(Genode::Capability<Genode::Region_map> region_map, Genode::Dataspace_capability md_cap)
	:
		ref_region_map(region_map), ref_managed_dataspace(md_cap), attachable_dataspaces(), context()
	{ }

	/**
	 * Find Region_map_info by using a specific Dataspace_capability
	 *
	 * \param cap Dataspace_capability of the corresponding managed dataspace
	 *
	 * \return Region_map_info with the corresponding Capability
	 */
	Region_map_info *find_by_cap(Genode::Dataspace_capability cap)
	{
		if(cap == ref_region_map)
			return this;
		Region_map_info *region_map_info = next();
		return region_map_info ? region_map_info->find_by_cap(cap) : 0;
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
	static constexpr bool verbose_debug = false;

	/**
	 * Reference Region_map_info to which this dataspace belongs
	 */
	Rtcr::Region_map_info        &ref_region_map_info;
	/**
	 * Dataspace which will be attached to / detached from the reference Region_map
	 */
	Genode::Dataspace_capability  dataspace;
	/**
	 * Starting address of the dataspace; it is local to the Region_map to which it will be attached
	 */
	Genode::addr_t                local_addr;
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
	 * \param region_map_info Reference Region_map_info which stores the Region_map to which the dataspace belongs
	 * \param ds_cap          Dataspace which will be attached to the Region_map
	 * \param local_addr      Address where the dataspace will be attached at
	 * \param size            Size of the Dataspace
	 * \param attached        Indicates whether the dataspace is attached in the Region_map
	 */
	Attachable_dataspace_info(Rtcr::Region_map_info &region_map_info, Genode::Dataspace_capability ds_cap,
			Genode::addr_t local_addr, Genode::size_t size)
	:
		ref_region_map_info(region_map_info), dataspace(ds_cap), local_addr(local_addr), size(size), attached(false)
	{ }

	/**
	 * Find Attachable_dataspace_info which contains the address addr
	 *
	 * \param addr Local address of the corresponding Region_map
	 *
	 * \return Attachable_dataspace_info which contains the local address addr
	 */
	Attachable_dataspace_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= local_addr) && (addr <= local_addr + size))
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
				Genode::log("Attaching dataspace ", dataspace.local_name(),
						" to managed dataspace ", ref_region_map_info.ref_managed_dataspace.local_name(),
						" on location ", Genode::Hex(local_addr), " (local to Region_map)");
			}

			// Attaching Dataspace to designated location
			Genode::addr_t addr =
					Genode::Region_map_client{ref_region_map_info.ref_region_map}.attach_at(dataspace, local_addr);

			// Dataspace was not attached on the right location
			if(addr != local_addr)
			{
				Genode::warning("Attachable_dataspace_info::attach Dataspace was not attached on its designated location!");
				Genode::warning("  designated", Genode::Hex(local_addr), " != attached=", Genode::Hex(addr));
			}

			// Mark as attached
			attached = true;
		}
		else
		{
			Genode::warning("Trying to attach an already attached Dataspace:",
					" DS ", dataspace.local_name(),
					" MD ", ref_region_map_info.ref_managed_dataspace.local_name(),
					" Loc ", Genode::Hex(local_addr));
		}
	}

	/**
	 * Detach dataspace and mark it as not attached
	 */
	void detach()
	{
		if(attached)
		{
			// Detaching Dataspace
			Genode::Region_map_client{ref_region_map_info.ref_region_map}.detach(local_addr);

			// Mark as detached
			attached = false;
		}
		else
		{
			Genode::warning("Trying to detach an already detached Dataspace:",
					" DS ", dataspace.local_name(),
					" RM ", ref_region_map_info.ref_region_map.local_name(),
					" Loc ", Genode::Hex(local_addr));
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
	Genode::Signal_receiver             &_receiver;
	/**
	 * List of region maps and their associated dataspaces
	 */
	Genode::List<Rtcr::Region_map_info> &_managed_dataspaces;

	/**
	 * Handle page fault signal by finding the faulting region map and attaching the designated dataspace on the faulting address
	 */
	void _handle_fault()
	{
		Genode::Region_map::State state;

		// Find first faulting Region map in the list of managed dataspaces
		Rtcr::Region_map_info *curr_md = _managed_dataspaces.first();
		for( ; curr_md; curr_md = curr_md->next())
		{
			Genode::Region_map_client rm_client {curr_md->ref_region_map};

			// If the region map is found, cancel the search and do not override the pointer to the corresponding Region_map_info
			if(rm_client.state().type != Genode::Region_map::State::READY)
			{
				state = rm_client.state();

				if(verbose_debug)
				{
				Genode::log("Handle fault: Managed dataspace ", curr_md->ref_managed_dataspace.local_name(), " state is ",
					state.type == Genode::Region_map::State::READ_FAULT  ? "READ_FAULT"  :
					state.type == Genode::Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
					state.type == Genode::Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
					" pf_addr=", Genode::Hex(state.addr));
				}

				break;
			}
		}

		// Find dataspace which contains the faulting address
		Rtcr::Attachable_dataspace_info *dataspace_info =
				curr_md->attachable_dataspaces.first()->find_by_addr(state.addr);

		// Check if a dataspace was found
		if(!dataspace_info)
		{
			Genode::warning("No designated dataspace for addr = ", state.addr,
					" in Region_map ", curr_md->ref_region_map.local_name());
			return;
		}

		// Attach found dataspace to its designated address
		dataspace_info->attach();
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
			Genode::List<Rtcr::Region_map_info> &managed_dataspaces)
	:
		Thread(env, "managed dataspace pager", 16*1024),
		_receiver(receiver), _managed_dataspaces(managed_dataspaces)
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
	Genode::Env                   &_env;
	/**
	 * Allocator for objects belonging to the monitoring of the managed dataspaces (e.g. Region_map_info and Attachable_dataspace_info)
	 */
	Genode::Allocator             &_md_alloc;

	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Genode::Ram_connection         _parent_ram;
	/**
	 * Connection to the parent Rm session for creating new Region_maps (usually core's Rm session)
	 */
	Genode::Rm_connection          _parent_rm;
	/**
	 * List of managed dataspaces (= Region maps) given to the client
	 */
	Genode::List<Region_map_info>  _managed_dataspaces;
	/**
	 * Receiver of page faults
	 */
	Genode::Signal_receiver        _receiver;
	/**
	 * Fault handler which marks and attaches dataspaces associated with the faulting address
	 */
	Rtcr::Fault_handler            _page_fault_handler;
	// TODO create one fault handler for all three Region maps (address space, stack area, linker area)


	/**
	 * Removes page fault handler from Region_map,
	 * deletes the Region_map_info and its Attachable_dataspace_infos, and
	 * removes the Region_map_info from managed_dataspaces
	 *
	 * \param rm_info Pointer to the Region_map_info to delete
	 */
	void _delete_rm_info(Region_map_info *rm_info)
	{
		// TODO Do the removing/deleting in the destructor of Region_map_info

		Attachable_dataspace_info *curr_ds = nullptr;

		// Remove page fault handler from Region_map
		_receiver.dissolve(&rm_info->context);

		// Delete Attachable_dataspace_infos from current Region_map_info
		while((curr_ds = rm_info->attachable_dataspaces.first()))
		{
			// Remove current Attachable_dataspace_info from the list
			rm_info->attachable_dataspaces.remove(curr_ds);

			// Delete current Attachable_dataspace_info from the allocator
			Genode::destroy(_md_alloc, curr_ds);
		}

		// Remove current Region_map_info from the list
		_managed_dataspaces.remove(rm_info);

		// Delete current Region_map_info from the allocator
		Genode::destroy(_md_alloc, rm_info);
	}

public:

	/**
	 * Constructor
	 *
	 * \param env      Environment for creating a session to parent's RAM service
	 * \param md_alloc Allocator for Region_map_info and Attachable_dataspace_info
	 * \param name     Label for parent's Ram session
	 */
	Ram_session_component(Genode::Env &env, Genode::Allocator &md_alloc, const char *name)
	:
		_env       (env),
		_md_alloc  (md_alloc),
		_parent_ram(env, name),
		_parent_rm (env),
		_managed_dataspaces(),
		_receiver(),
		_page_fault_handler(env, _receiver, _managed_dataspaces)
	{
		_page_fault_handler.start();
		if(verbose_debug) Genode::log("Ram_session_component created");
	}

	/**
	 * Destructor
	 */
	~Ram_session_component()
	{
		Region_map_info *curr_md = nullptr;

		// Remove page fault handler from all Region_maps,
		// delete all Region_map_infos with their Attachable_dataspaces_infos
		while((curr_md = _managed_dataspaces.first()))
		{
			// Implicitly removes the first Region_map_info from the managed dataspaces list
			_delete_rm_info(curr_md);
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
			Genode::log("Ram::alloc(size=", Genode::Hex(size),")");
		}

		enum { GRANULARITY = 4096 };
		Genode::size_t ds_size = static_cast<Genode::size_t>(GRANULARITY);

		// TODO change from pages to allow arbitrary dataspace size (but they must be multiple of a pagesize)
		// Compute the number of dataspaces which the managed dataspace shall contain
		Genode::size_t rest_page = size % GRANULARITY;
		Genode::size_t num_pages = (size / GRANULARITY) + (rest_page == 0 ? 0 : 1);

		// Create a Region map; if Rm_session is out of ram_quota, upgrade it
		Genode::Capability<Genode::Region_map> new_region_map =
			Genode::retry<Genode::Rm_session::Out_of_metadata>(
				[&] () { return _parent_rm.create(num_pages*GRANULARITY); },
				[&] ()
				{
					char args[Genode::Parent::Session_args::MAX_SIZE];
					Genode::snprintf(args, sizeof(args), "ram_quota=%u", 64*1024);
					_env.parent().upgrade(_parent_rm, args);
				});

		// Create a Region_map_client for the new Region_map for convenience
		Genode::Region_map_client new_rm_client{new_region_map};

		// Create Region_map_info which contains a list of corresponding dataspaces
		Rtcr::Region_map_info *rm_info = new (_md_alloc)
				Rtcr::Region_map_info(new_region_map, new_rm_client.dataspace());

		// Insert into _managed_dataspaces
		_managed_dataspaces.insert(rm_info);

		// Set our pagefault handler for the Region_map with its own context
		new_rm_client.fault_handler(_receiver.manage(&(rm_info->context)));

		// Allocate memory for the new Region_map and Store the information in Region_map_info
		for(Genode::size_t i = 0; i < num_pages; i++)
		{
			Genode::Dataspace_capability ds_cap;

			// Try to allocate a dataspace which will be associated with the new region_map
			try
			{
				// eager creation of attachments
				// XXX lazy creation could be an optimization (create dataspaces when they are used)
				ds_cap = _parent_ram.alloc(GRANULARITY, cached);
			}
			catch(Genode::Ram_session::Quota_exceeded)
			{
				Genode::error("_parent_ram has no memory!");
				return Genode::Capability<Genode::Ram_dataspace>();
			}
			// Prepare arguments for creating a Attachable_dataspace_info
			Genode::addr_t local_addr = ds_size * i;

			// Create an Attachable_dataspace_info
			Rtcr::Attachable_dataspace_info *att_ds_info =
					new (_md_alloc) Rtcr::Attachable_dataspace_info(*rm_info, ds_cap, local_addr, ds_size);
			// Attach the dataspace into the corresponding Region_map on the corresponding address
			//att_ds_info->attach();

			// Insert into Region_map_infos list of dataspaces
			rm_info->attachable_dataspaces.insert(att_ds_info);
		}

		if(verbose_debug)
		{
			Genode::log("  Created a managed dataspace (", rm_info->ref_managed_dataspace.local_name(), ")",
					" containing ", num_pages, " Dataspaces with the size of ", ds_size);
		}

		// Return the stored managed dataspace capability of the Region_map as a Ram_dataspace_capability
		return Genode::static_cap_cast<Genode::Ram_dataspace>(rm_info->ref_managed_dataspace);
	}

	/**
	 * Frees the managed dataspace and deletes it from _managed_dataspaces
	 *
	 * \param ds Capability of the managed dataspace
	 */
	void free(Genode::Ram_dataspace_capability ds) override
	{
		if(verbose_debug) Genode::log("Ram::free()");

		// Find the Region_map_info which represents the Region_map with the passed Dataspace_capability
		Region_map_info *region_map_info = _managed_dataspaces.first()->find_by_cap(ds);

		// Check whether a corresponding Region_map_info exists
		if(!region_map_info)
		{
			Genode::warning("Region_map not found in Ram::free(). Capability: ", ds.local_name(),
					" not in managed dataspace list.");
			return;
		}

		// Delete the found Region_map_info and remove it from managed_dataspaces
		_delete_rm_info(region_map_info);

		_parent_ram.free(ds);
	}

	int ref_account(Genode::Ram_session_capability ram_session) override
	{
		if(verbose_debug)
		{
			Genode::log("Ram::ref_account(ref=", ram_session.local_name(), ")");
		}

		return _parent_ram.ref_account(ram_session);
	}

	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override
	{
		if(verbose_debug)
		{
			Genode::log("Ram::transfer_quota(to=", ram_session.local_name(), ")");
		}
		return _parent_ram.transfer_quota(ram_session, amount);
	}

	Genode::size_t quota() override
	{
		if(verbose_debug) Genode::log("Ram::quota()");
		return _parent_ram.quota();
	}

	Genode::size_t used() override
	{
		if(verbose_debug) Genode::log("Ram::used()");
		return _parent_ram.used();
	}

};

#endif /* _RTCR_RAM_SESSION_COMPONENT_H_ */
