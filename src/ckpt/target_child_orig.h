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
/*#include <ram_session/connection.h>
#include <cpu_session/connection.h>
#include <pd_session/connection.h>
#include <region_map/client.h>*/

/* Rtcr includes */
#include "pd_session_component.h"
#include "cpu_session_component.h"
#include "ram_session_component.h"

namespace Rtcr {
	class Target_child;
	using namespace Genode;
}

class Rtcr::Target_child : public Child_policy
{
private:
	static constexpr bool verbose = true;
	/**
	 * Checkpointer's environment
	 */
	Env        &_env;
	Allocator  &_md_alloc;
	/**
	 * Child's Entrypoint
	 */
	Entrypoint &_resource_ep;
	const char *_unique_name;

	struct Pd_manager
	{
		const char *label;
		Rtcr::Pd_session_component pd;

		Pd_manager(Env &env, Allocator &md_alloc, const char *unique_name)
		:
			label(unique_name),
			pd(env, md_alloc, label)
		{
			log("Pd session managed");
		}
	} _pd_manager;

	struct Cpu_manager
	{
		Rtcr::Cpu_session_component cpu;

		Cpu_manager(Env &env, Allocator &md_alloc, Pd_session_capability parent_pd_cap)
		:
			cpu(env, md_alloc, parent_pd_cap)
		{
			log("Cpu session managed");
		}
	} _cpu_manager;

	struct Ram_manager
	{
		const size_t donate_quota = 1*1024*1024;
		Rtcr::Ram_session_component ram;

		Ram_manager(Env &env, Allocator &md_alloc)
		:
			ram(env, md_alloc)
		{
			log("ref_account: ", ram.ref_account(env.ram_session_cap()));
			log("transfer result: ", env.ram().transfer_quota(ram.cap(), donate_quota));
			log("Ram quota ", ram.quota());
			log("Ram session managed");
		}
	} _ram_manager;

	Rom_connection        _elf;
	Child::Initial_thread _initial_thread;
	Service_registry      _parent_services;
	Region_map_client     _address_space;
	Child                 _child;

public:

	/**
	 * Constructor
	 */
	Target_child(Env &env, Allocator &md_alloc, Entrypoint &ep,
			const char *unique_name)
	:
		_env(env), _md_alloc(md_alloc), _resource_ep(ep),
		_unique_name(unique_name),
		_pd_manager(_env, _md_alloc, _unique_name),
		_cpu_manager(_env, _md_alloc, _pd_manager.pd.parent_pd_cap()),
		_ram_manager(_env, _md_alloc),
		_elf(unique_name),
		_initial_thread(_cpu_manager.cpu, _pd_manager.pd.parent_pd_cap(),_unique_name),
		_parent_services(),
		_address_space(_pd_manager.pd.address_space()),
		_child(_elf.dataspace(), Dataspace_capability(),
				_pd_manager.pd.cap(), _pd_manager.pd,
				_ram_manager.ram.cap(), _ram_manager.ram,
				_cpu_manager.cpu.cap(), _initial_thread,
				_env.rm(), _address_space, _env.ep().rpc_ep(), *this)
	{
		if(verbose) log("Target_child created");
	}

	const char *name() const { return _unique_name; }

	Service *resolve_session_request(const char *service_name, const char *args)
	{
		Service *service = nullptr;

		if(!(service = _parent_services.find(service_name)))
		{
			service = new (_md_alloc) Parent_service(service_name);
			_parent_services.insert(service);
		}

		return service;
	}

	void filter_session_args(const char *service, char *args, size_t args_len)
	{
		/* define session label for sessions forwarded to our parent */
		Arg_string::set_arg_string(args, args_len, "label", _unique_name);
	}

};

#endif /* _RTCR_TARGET_CHILD_H_ */
