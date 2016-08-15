/*
 * \brief Child creation
 * \author Denis Huber
 * \date 2016-08-04
 */

#ifndef _RTCR_TARGET_CHILD_H_
#define _RTCR_TARGET_CHILD_H_

/* Genode includes */
#include <base/child.h>
#include <base/env.h>
#include <rom_session/connection.h>

/* Rtcr includes */
#include "pd_session_component.h"
#include "cpu_session_component.h"
#include "ram_session_component.h"

namespace Rtcr {
	class Target_child;
}

class Rtcr::Target_child : Genode::Child_policy
{
private:
	static constexpr bool verbose = true;
	/**
	 * Checkpointer's environment
	 */
	Genode::Env        &_env;
	/**
	 * Entrypoint for child's virtualized sessions and for child's creation
	 */
	Genode::Entrypoint &_resource_ep;
	/**
	 * Allocator for child's virtualized sessions
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Child's Entrypoint
	 */
	const char *_unique_name;

	struct Pd_manager
	{
		Rtcr::Pd_session_component pd;

		Pd_manager(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc, const char *unique_name)
		:
			pd(env, ep, md_alloc, unique_name)
		{
			Genode::log("Pd session managed");
		}
	} _pd_manager;

	struct Cpu_manager
	{
		Rtcr::Cpu_session_component cpu;

		Cpu_manager(Genode::Entrypoint &ep, Genode::Allocator &md_alloc, Genode::Pd_session_capability parent_pd_cap)
		:
			cpu(ep, md_alloc, parent_pd_cap)
		{
			Genode::log("Cpu session managed");
		}
	} _cpu_manager;

	struct Ram_manager
	{
		const Genode::size_t donate_quota = 1*1024*1024;
		Rtcr::Ram_session_component ram;

		Ram_manager(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc)
		:
			ram(ep, md_alloc)
		{
			ram.ref_account(env.ram_session_cap());
			env.ram().transfer_quota(ram.parent_ram_cap(), donate_quota);
			Genode::log("Ram session managed");
		}
	} _ram_manager;

	Genode::Rom_connection _rom;
	Genode::Child::Initial_thread _initial_thread;
	Genode::Region_map_client _address_space;
	Genode::Service_registry &_parent_services;
	Genode::Service_registry  _local_services;
	Genode::Service_registry  _child_services;
	Genode::Child _child;

public:

	/**
	 * Constructor
	 */
	Target_child(Genode::Env &env, Genode::Entrypoint &resource_ep, Genode::Allocator &md_alloc,
			Genode::Service_registry &parent_services, const char *unique_name)
	:
		_env(env), _resource_ep(resource_ep), _md_alloc(md_alloc),
		_unique_name(unique_name),
		_pd_manager(_env, _resource_ep, _md_alloc, _unique_name),
		_cpu_manager(_resource_ep, _md_alloc, _pd_manager.pd.parent_pd_cap()),
		_ram_manager(_env, _resource_ep, _md_alloc),
		_rom(_unique_name),
		_initial_thread(_cpu_manager.cpu, _pd_manager.pd.cap(), _unique_name),
		_address_space(_pd_manager.pd.address_space()),
		_parent_services(parent_services),
		_child(_rom.dataspace(), Genode::Dataspace_capability(),
				_pd_manager.pd.cap(),   _pd_manager.pd,
				_ram_manager.ram.parent_ram_cap(), _ram_manager.ram,
				_cpu_manager.cpu.cap(), _initial_thread,
				_env.rm(), _address_space, _resource_ep.rpc_ep(), *this)
	{
		if(verbose) Genode::log("Target_child created");
	}

	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return _unique_name; }

	Genode::Service *resolve_session_request(const char *service_name, const char *args)
	{
		//Genode::log(service, " ", args);

		Genode::Service *service = 0;

		// Service is implemented locally?
		service = _local_services.find(service_name);
		if(service)
			return service;

		// Service known from parent?
		service = _parent_services.find(service_name);
		if(service)
			return service;

		// Service known from child?
		service = _child_services.find(service_name);
		if(service)
			return service;

		// Service not known: Assume parent provides it and fill parent service registry on demand
		if(!service)
		{
			service = new (_md_alloc) Genode::Parent_service(service_name);
			_parent_services.insert(service);
			if(verbose) Genode::log("Inserted service \"", service_name, "\" into parent_services");
		}

return service;
	}

	void filter_session_args(const char *service, char *args,
	                         Genode::size_t args_len)
	{
		Genode::Arg_string::set_arg_string(args, args_len, "label", "sheep_counter");
	}
};

#endif /* _RTCR_TARGET_CHILD_H_ */
