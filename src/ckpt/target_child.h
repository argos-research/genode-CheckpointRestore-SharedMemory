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
#include <ram_session/connection.h>
#include <cpu_session/connection.h>
#include <pd_session/connection.h>
#include <region_map/client.h>

/* Rtcr includes */
#include "pd_session_component.h"
#include "cpu_root.h"

namespace Rtcr {
	class Target_child;
}

class Rtcr::Target_child : public Genode::Child_policy
{
private:

	Genode::Env        &_env;
	Genode::Allocator  &_md_alloc;
	Genode::Entrypoint &_ep;
	const char         *_unique_name;

	struct Resources
	{
		Genode::Ram_connection ram;
		Rtcr::Pd_session_component pd;
		Rtcr::Cpu_root cpu_root;
		Genode::Ram_session_client cpu_session;

		Resources(Genode::Env &env, Genode::Allocator &md_alloc, const char *label)
		:
			ram(),
			pd(env, md_alloc, label),
			//pd(label),
			cpu_root(label, env, md_alloc, pd.core_pd_cap())
		{
			enum { CHILD_QUOTA = 1*1024*1024 };
			ram.ref_account(env.ram_session_cap());
			env.ram().transfer_quota(ram.cap(), CHILD_QUOTA);
		}
	} _resources;

	/**
	 * Initial thread of child
	 */
	Genode::Child::Initial_thread  _initial_thread;
	Genode::Service_registry   _parent_services;
	Genode::Rom_connection     _elf;
	Genode::Region_map_client  _address_space { _resources.pd.address_space() };
	Genode::Child              _child;

public:

	/**
	 * Constructor
	 */
	Target_child(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &ep,
			const char *unique_name)
	:
		_env(env), _md_alloc(md_alloc), _ep(ep),
		_unique_name(unique_name),
		_resources(_env, _md_alloc, _unique_name),
		_initial_thread(_resources.cpu, _resources.pd.cap(), _unique_name),
		_parent_services(),
		_elf(unique_name),
		_child(_elf.dataspace(), Genode::Dataspace_capability(),
		       _resources.pd.cap(),  _resources.pd,
		       _resources.ram, _resources.ram,
		       _resources.cpu, _initial_thread,
		       _env.rm(), _address_space, _ep.rpc_ep(), *this)
	{ }

	const char *name() const { return _unique_name; }

	Service *resolve_session_request(const char *service_name, const char *args)
	{
		Service *service = nullptr;

		if(!(service = _parent_services.find(service_name)))
		{
			service = new (_md_alloc) Genode::Parent_service(service_name);
			_parent_services.insert(service);
		}

		return service;
	}

	void filter_session_args(const char *service, char *args, Genode::size_t args_len)
	{
		/* define session label for sessions forwarded to our parent */
		Genode::Arg_string::set_arg_string(args, args_len, "label", _unique_name);
	}

};

#endif /* _RTCR_TARGET_CHILD_H_ */
