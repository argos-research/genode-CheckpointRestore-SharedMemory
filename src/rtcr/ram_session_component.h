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
	class Fault_handler;
	struct Region_map_info;
	struct Attachable_dataspace_info;
	class Ram_session_component;
}

class Rtcr::Fault_handler : public Genode::Thread
{
private:
	Genode::Signal_receiver &_receiver;
	Genode::List<Rtcr::Region_map_info> &_managed_dataspaces;

	void _handle_fault()
	{
		Genode::Region_map::State state;

		// Current managed_dataspace
		Rtcr::Region_map_info *curr_md = _managed_dataspaces.first();

		for( ; curr_md; curr_md = curr_md->next())
		{
			// found!
			if(curr_md->ref_region_map.state().type != Genode::Region_map::State::READY)
			{
				state = curr_md->ref_region_map.state();
				break;
			}
		}

		Rtcr::Attachable_dataspace_info *curr_ds = curr_md->parent_dataspaces.first();
		for( ; curr_ds; curr_ds = curr_ds->next())
		{

		}

		curr_md->ref_region_map.attach_at();
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

struct Rtcr::Region_map_info : public Genode::List<Region_map_info>::Element
{
	Genode::Region_map &ref_region_map;
	Genode::List<Rtcr::Attachable_dataspace_info> parent_dataspaces;

	Region_map_info() { }
};

struct Rtcr::Attachable_dataspace_info : public Genode::List<Attachable_dataspace_info>::Element
{
	Genode::Dataspace_capability parent_dataspace;
	Rtcr::Region_map_info &ref_region_map_info;
	Genode::addr_t local_addr;
	Genode::size_t size;
	bool attached;

	Attachable_dataspace_info() { }
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


public:

	Ram_session_component(Genode::Env &env, Genode::Allocator &md_alloc, const char *name)
	:
		_env       (env),
		_md_alloc  (md_alloc),
		_parent_ram(env, name),
		_parent_rm (env)
	{
		if(verbose_debug) Genode::log("Ram_session_component created");
	}

	~Ram_session_component()
	{
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

		Genode::size_t rest_page = size % GRANULARITY;
		Genode::size_t num_pages = (size / GRANULARITY) + (rest_page == 0 ? 0 : 1);

		Genode::Capability<Genode::Region_map> new_region_map;

		// Create a Region map; if out of ram_quota, upgrade session
		new_region_map = Genode::retry<Genode::Rm_session::Out_of_metadata>(
				[&] () { return _parent_rm.create(num_pages*GRANULARITY); },
				[&] ()
				{
					char args[Genode::Parent::Session_args::MAX_SIZE];
					Genode::snprintf(args, sizeof(args), "ram_quota=%u", 64*1024);
					_env.parent().upgrade(_parent_rm, args);
				});
		// TODO store new region map

		Genode::Region_map_client rm_client { new_region_map };

		// Allocate and attach memory to Region_map
		for(Genode::size_t i = 0; i < num_pages; i++)
		{
			try
			{
				// eager creation of attachments
				// XXX lazy creation could be an optimization
				Genode::Region_map::Local_addr addr = rm_client.attach(_parent_ram.alloc(GRANULARITY, cached));
				// TODO store dataspaces
			}
			catch(Genode::Ram_session::Quota_exceeded)
			{
				Genode::error("_parent_ram has no memory!");
				return Genode::Capability<Genode::Ram_dataspace>();
			}
		}

		return Genode::static_cap_cast<Genode::Ram_dataspace>(rm_client.dataspace());
	}

	void free(Genode::Ram_dataspace_capability ds) override
	{
		if(verbose_debug) Genode::log("Ram::free()");
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
