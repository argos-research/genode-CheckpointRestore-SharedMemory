/*
 * \brief Intercepting Ram session
 * \author Denis Huber
 * \date 2016-08-12
 */

#ifndef _RTCR_RAM_SESSION_COMPONENT_H_
#define _RTCR_RAM_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <ram_session/connection.h>
#include <rm_session/connection.h>
#include <dataspace/client.h>


namespace Rtcr {
	class Ram_session_component;
}

class Rtcr::Ram_session_component : public Genode::Rpc_object<Genode::Ram_session>
{
private:
	static constexpr bool verbose_debug = false;

	Genode::Env        &_env;
	Genode::Allocator  &_md_alloc;

	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Genode::Ram_connection _parent_ram;
	Genode::Rm_connection  _parent_rm;

	struct Dataspace_info : Genode::List<Dataspace_info>::Element
	{
		Dataspace_info() { }
	};

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
		if(verbose_debug) Genode::log("Ram::alloc()");
		enum { GRANULARITY = 4096 };

		Genode::size_t rest_page = size % GRANULARITY;
		Genode::size_t num_pages = (size / GRANULARITY) + (rest_page == 0 ? 0 : 1);
		//Genode::log("r: ", rest_page, " n: ", num_pages);

		Genode::Capability<Genode::Region_map> new_region_map;

		// try to create a Region map
		try
		{
			new_region_map = _parent_rm.create(num_pages*GRANULARITY);
		}
		// Out of metadata => upgrade and create Region map
		catch(Genode::Region_map::Out_of_metadata)
		{
			char args[Genode::Parent::Session_args::MAX_SIZE];
			Genode::snprintf(args, sizeof(args), "ram_quota=%u", 64*1024);
			_env.parent().upgrade(_parent_rm, args);

			new_region_map = _parent_rm.create(num_pages*GRANULARITY);
		}

		Genode::Region_map_client rm_out { new_region_map };
		//Genode::log("Region_map created.");

		for(Genode::size_t i = 0; i < num_pages; i++)
		{
			try
			{
				rm_out.attach(_parent_ram.alloc(GRANULARITY, cached));
			}
			catch(Genode::Allocator::Out_of_memory)
			{
				Genode::error("_parent_ram has no memory!");
				return Genode::Capability<Genode::Ram_dataspace>();
			}
		}

		//Genode::log("Region_map filled.");

		return Genode::static_cap_cast<Genode::Ram_dataspace>(rm_out.dataspace());
	}

	void free(Genode::Ram_dataspace_capability ds) override
	{
		if(verbose_debug) Genode::log("Ram::free()");
		_parent_ram.free(ds);
	}

	int ref_account(Genode::Ram_session_capability ram_session) override
	{
		if(verbose_debug) Genode::log("Ram::ref_account()");
		return _parent_ram.ref_account(ram_session);
	}

	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override
	{
		if(verbose_debug) Genode::log("Ram::transfer_quota()");
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
