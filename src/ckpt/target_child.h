/*
 * \brief Child creation
 * \author Denis Huber
 * \date 2016-08-04
 */

#ifndef _RTCR_TARGET_CHILD_H_
#define _RTCR_TARGET_CHILD_H_

/* Genode includes */
#include <base/env.h>
#include <base/child.h>
#include <rom_session/connection.h>

/* Rtcr includes */
#include "pd_session_component.h"
#include "cpu_session_component.h"
#include "ram_session_component.h"

namespace Rtcr {
	class Target_child;
}

class Rtcr::Target_child : public Genode::Child_policy
{
private:
	static constexpr bool verbose_debug = true;
	Genode::String<32>  _name;
	Genode::Env        &_env;
	Genode::Allocator  &_md_alloc;
	Genode::Entrypoint  _resources_ep;
	Genode::Entrypoint  _child_ep;
	struct Resources
	{
		Genode::Entrypoint          &ep;
		Rtcr::Pd_session_component   pd;
		Rtcr::Cpu_session_component  cpu;
		Rtcr::Ram_session_component  ram;
		Genode::Rom_connection       rom;
		Resources(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc, const char *name)
		:
			ep (ep),
			pd (env, md_alloc, this->ep, name),
			cpu(env, md_alloc, pd.parent_cap(), name),
			ram(env, md_alloc, name),
			rom(env, name)
		{
			this->ep.manage(pd);
			this->ep.manage(cpu);
			this->ep.manage(ram);

			Genode::size_t donate_quota = 100*1024*1024;
			ram.ref_account(env.ram_session_cap());
			env.ram().transfer_quota(ram.parent_cap(), donate_quota);
		}

		~Resources()
		{
			ep.dissolve(ram);
			ep.dissolve(cpu);
			ep.dissolve(pd);
		}
	} _resources;

	Genode::Child::Initial_thread  _initial_thread;
	Genode::Region_map_client      _address_space;
	Genode::Service_registry      &_parent_services;
	Genode::Service_registry       _local_services;
	Genode::Service_registry       _child_services;
	Genode::Child                  _child;

public:

	/**
	 * Constructor
	 */
	Target_child(Genode::Env &env, Genode::Allocator &md_alloc,
			Genode::Service_registry &parent_services, const char *name)
	:
		_name           (name),
		_env            (env),
		_md_alloc       (md_alloc),
		_resources_ep   (_env, 16*1024, "resources ep"),
		_child_ep       (_env, 16*1024, "child ep"),
		_resources      (_env, _resources_ep, _md_alloc, _name.string()),
		_initial_thread (_resources.cpu, _resources.pd.cap(), _name.string()),
		_address_space  (_resources.pd.address_space()),
		_parent_services(parent_services),
		_local_services (),
		_child_services (),
		_child(_resources.rom.dataspace(), Genode::Dataspace_capability(),
				_resources.pd.cap(),  _resources.pd,
				_resources.ram.cap(), _resources.ram,
				_resources.cpu.cap(), _initial_thread,
				_env.rm(), _address_space, _child_ep.rpc_ep(), *this)
	{
		if(verbose_debug) Genode::log("Target_child \"", _name.string(), "\" created.");
	}

	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return _name.string(); }

	Genode::Service *resolve_session_request(const char *service_name, const char *args)
	{
		if(verbose_debug) Genode::log("Session request: ", service_name, " ", args);

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
			if(verbose_debug) Genode::log("  Inserted service into parent_services");
		}

		return service;
	}

	void filter_session_args(const char *service, char *args, Genode::size_t args_len)
	{
		Genode::Arg_string::set_arg_string(args, args_len, "label", _name.string());
	}
};

#endif /* _RTCR_TARGET_CHILD_H_ */
