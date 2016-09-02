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
};

/**
 * Struct which holds information about a dataspace:
 *
 * - Which Region_map does it belong to
 * - where shall it be attached,
 * - how big is it, and
 * - is it attached in the referenced Region_map
 */
struct Rtcr::Attachable_dataspace_info : public Genode::List<Attachable_dataspace_info>::Element
{
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
			Genode::addr_t local_addr, Genode::size_t size, bool attached = false)
	:
		ref_region_map_info(region_map_info), dataspace(ds_cap), local_addr(local_addr), size(size), attached(attached)
	{
		if(attached) attach();
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
			Genode::addr_t addr =
					Genode::Region_map_client{ref_region_map_info.ref_region_map}.attach_at(dataspace, local_addr);

			if(addr != local_addr)
			{
				Genode::warning("Attachable_dataspace_info::attach Returned address is unequal to local_addr");
				Genode::warning("  ", Genode::Hex(addr), " != ", Genode::Hex(local_addr));
			}

			attached = true;
		}
	}

	/**
	 * Detach dataspace and mark it as not attached
	 */
	void detach()
	{
		if(attached)
		{
			Genode::Region_map_client{ref_region_map_info.ref_region_map}.detach(local_addr);

			attached = false;
		}
	}
};

class Rtcr::Fault_handler : public Genode::Thread
{
private:
	Genode::Signal_receiver             &_receiver;
	Genode::List<Rtcr::Region_map_info> &_managed_dataspaces;

	void _handle_fault()
	{
		Genode::Region_map::State state;

		// Current managed_dataspace
		Rtcr::Region_map_info *curr_md = _managed_dataspaces.first();

		for( ; curr_md; curr_md = curr_md->next())
		{
			Genode::Region_map_client rm_client {curr_md->ref_region_map};
			// found!
			if(rm_client.state().type != Genode::Region_map::State::READY)
			{
				state = rm_client.state();
				break;
			}
		}

		// Find dataspace which includes the faulting address
		Rtcr::Attachable_dataspace_info *dataspace_info =
				curr_md->attachable_dataspaces.first()->find_by_addr(state.addr);

		// Check if a dataspace was found
		if(!dataspace_info)
		{
			Genode::warning("No designated dataspace for addr = ", state.addr,
					" in Region_map ", curr_md->ref_region_map.local_name());
			return;
		}

		// Attach found dataspace on its designated address
		dataspace_info->attach();
	}

public:
	Fault_handler(Genode::Env &env, Genode::Signal_receiver &receiver,
			Genode::List<Rtcr::Region_map_info> &managed_dataspaces)
	:
		Thread(env, "managed dataspace pager", 16*1024),
		_receiver(receiver), _managed_dataspaces(managed_dataspaces)
	{ }

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

class Rtcr::Ram_session_component : public Genode::Rpc_object<Genode::Ram_session>
{
private:
	static constexpr bool verbose_debug = true;

	Genode::Env        &_env;
	Genode::Allocator  &_md_alloc;

	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Genode::Ram_connection _parent_ram;
	Genode::Rm_connection  _parent_rm;

	/**
	 * List of managed dataspaces (=Region maps) given to the client
	 */
	Genode::List<Region_map_info> _managed_dataspaces;
	/**
	 * Fault handler which marks and attaches dataspaces associated with the faulting address
	 */
	Genode::Signal_receiver       _receiver;
	Rtcr::Fault_handler           _page_fault_handler;


public:

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

	~Ram_session_component()
	{
		// TODO do for each Region_map _receiver.dissolve(&_context);
		if(verbose_debug) Genode::log("Ram_session_component destroyed");
	}

	Genode::Ram_session_capability parent_cap()
	{
		return _parent_ram.cap();
	}

	/***************************
	 ** Ram_session interface **
	 ***************************/

	Genode::Ram_dataspace_capability alloc(Genode::size_t size, Genode::Cache_attribute cached) override
	{
		if(verbose_debug)
		{
			Genode::log("Ram::alloc()");
			Genode::log("  size=", size);
		}

		enum { GRANULARITY = 4096 };

		// TODO change from pages to allow arbitrary dataspace size (but they must be multiple of a pagesize)
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
			Genode::addr_t local_addr = static_cast<Genode::addr_t>(GRANULARITY) * i;
			Genode::size_t size       = static_cast<Genode::size_t>(GRANULARITY);

			// Create an attachable dataspace info
			// which implicitly attaches the dataspace into the corresponding Region_map on the corresponding address
			Rtcr::Attachable_dataspace_info *att_ds_info =
					new (_md_alloc) Rtcr::Attachable_dataspace_info(*rm_info, ds_cap, local_addr, size, true);

			// Insert into the dataspace list of the corresponding Region_map
			rm_info->attachable_dataspaces.insert(att_ds_info);
		}

		// Return the stored managed dataspace capability of the Region_map as a Ram_dataspace_capability
		return Genode::static_cap_cast<Genode::Ram_dataspace>(rm_info->ref_managed_dataspace);
	}

	void free(Genode::Ram_dataspace_capability ds) override
	{
		if(verbose_debug) Genode::log("Ram::free()");
		// TODO delete/destroy the Region_map corresponding to the capability
		_parent_ram.free(ds);
	}

	int ref_account(Genode::Ram_session_capability ram_session) override
	{
		if(verbose_debug)
		{
			Genode::log("Ram::ref_account()");
			Genode::log("  ref: ", ram_session.local_name());
		}

		return _parent_ram.ref_account(ram_session);
	}

	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override
	{
		if(verbose_debug)
		{
			Genode::log("Ram::transfer_quota()");
			Genode::log("  to: ", ram_session.local_name());
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
