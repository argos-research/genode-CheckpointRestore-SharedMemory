/*
 * \brief Child creation
 * \author Denis Huber
 * \date 2016-08-04
 */

/* Genode includes */
#include <base/child.h>
#include <base/env.h>
#include <rom_session/connection.h>
#include <ram_session/connection.h>
#include <cpu_session/connection.h>
#include <region_map/client.h>

/* Rtcr includes */
#include "pd_session_component.h"

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
		Genode::Cpu_connection cpu;

		Resources(Genode::Env &env, Genode::Allocator &md_alloc,
				Genode::Entrypoint &ep, const char *label)
		:
			ram(),
			pd(env, md_alloc, ep, label),
			cpu()
		{
			enum { CHILD_QUOTA = 1*1024*1024 };
			ram.ref_account(env.ram_session_cap());
			env.ram().transfer_quota(ram.cap(), CHILD_QUOTA);
		}
	} _resources;

	Genode::Child::Initial_thread  _initial_thread;

	/**
	 * Entrypoint for serving the root interfaces of the services provided
	 * by the child and announced towards the parent of GDB monitor
	 */
	Genode::Entrypoint        &_root_ep;
	Genode::Service_registry   _parent_services;
	Genode::Service_registry   _child_services;
	Genode::Service_registry   _local_services;
	Genode::Rom_connection     _elf;
	Genode::Region_map_client  _address_space { _resources.pd.address_space() };
	Genode::Child              _child;

	// TODO: create a session capabilities need a root component
	// Pd session needs a root component too and session creation too?
	// TODO: activate entrypoint??
/*
	Cpu_session_capability _get_cpu_session_cap()
	{
		_entrypoint.manage(&_cpu_root);
		char args[64];
		Genode::snprintf(args, sizeof(args), "ram_quota=64K, label=\"%s\"", _unique_name);
		return static_cap_cast<Cpu_session>(_cpu_root.session(args, Affinity()));
	}
*/
public:

	/**
	 * Constructor
	 *
	 * \param ep Entrypoint for child's resources
	 * \param root_ep Entrypoint for child's root interfaces (i.e. services)
	 */
	Target_child(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &ep,
			Genode::Entrypoint &root_ep,
			const char *unique_name)
	:
		_env(env), _md_alloc(md_alloc), _ep(ep),
		_unique_name(unique_name),
		_resources(_env, _md_alloc, _ep, _unique_name),
		_initial_thread(_resources.cpu, _resources.pd.cap(), _unique_name),
		_root_ep(root_ep),
		_parent_services(), // TODO: Needs reference
		_child_services(),
		_local_services(),
		_elf(unique_name),
		_child(_elf.dataspace(), Genode::Dataspace_capability(),
		       _resources.pd.cap(),  _resources.pd,
		       _resources.ram, _resources.ram,
		       _resources.cpu, _initial_thread,
		       _env.rm(), _address_space, _ep.rpc_ep(), *this)
	{ }

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
		}

		return service;
	}

	bool announce_service(const char             *name,
	                      Genode::Root_capability root,
	                      Genode::Allocator      *alloc,
	                      Genode::Server         *server)
	{
		// Store child's root capability and announce the service to the parent
		_child_services.insert(new (alloc) Genode::Child_service(name, root, server));
		_env.parent().announce(name, root);
		return true;
	}

	void filter_session_args(const char *service, char *args,
	                         Genode::size_t args_len)
	{
		//Genode::Arg_string::set_arg_string(args, args_len, "label", "sheep_counter");
	}
};
