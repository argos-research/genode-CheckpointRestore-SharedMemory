/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-08-04
 */

#ifndef _RTCR_TARGET_CHILD_H_
#define _RTCR_TARGET_CHILD_H_

/* Genode includes */
#include <base/env.h>
#include <base/child.h>
#include <rom_session/connection.h>
#include <base/service.h>

/* Rtcr includes */
#include "intercept/cpu_session_component.h"
#include "intercept/pd_session_component.h"
#include "intercept/ram_session_component.h"
#include "intercept/rm_session.h"

namespace Rtcr {
	class Target_child;

	constexpr bool child_verbose_debug = true;
}

/**
 * Encapsulates the policy and creation of the child
 */
class Rtcr::Target_child : public Genode::Child_policy
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = child_verbose_debug;

	/**
	 * Child's unique name and filename of child's rom module
	 */
	Genode::String<32>  _name;
	/**
	 * Local environment
	 */
	Genode::Env        &_env;
	/**
	 * Local allocator
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Indicator whether to use incremental checkpointing
	 */
	const bool          _use_inc_ckpt;
	/**
	 * Entrypoint for managing child's resource-sessions (PD, CPU, RAM)
	 */
	Genode::Entrypoint  _resources_ep;
	/**
	 * Entrypoint for child's creation
	 */
	Genode::Entrypoint  _child_ep;
	/**
	 * Granularity for incremental checkpointing in a multiple of pagesize
	 */
	Genode::size_t      _granularity;
	/**
	 * Child's resources
	 */
	struct Resources
	{
		/**
		 * Entrypoint for managing the custom resources
		 */
		Genode::Entrypoint          &ep;
		/**
		 * Custom Pd Rpc_object
		 */
		Rtcr::Pd_session_component   pd;
		/**
		 * Custom Cpu Rpc_object
		 */
		Rtcr::Cpu_session_component  cpu;
		/**
		 * Custom Ram Rpc_object
		 */
		Rtcr::Ram_session_component  ram;
		/**
		 * Parent's Rom session (usually from core)
		 */
		Genode::Rom_connection       rom;
		/**
		 * Constructor
		 *
		 * \param env         Local environment
		 * \param ep          Entrypoint for child's resources
		 * \param md_alloc    Local allocator
		 * \param name        Child's unique name and also name of the rom module
		 * \param granularity Granularity for incremental checkpointing in multiple of pagesize
		 */
		Resources(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc, const char *name,
				bool use_inc_ckpt = true, Genode::size_t granularity = 1)
		:
			ep  (ep),
			pd  (env, md_alloc, this->ep, name),
			cpu (env, md_alloc, this->ep, pd.parent_cap(), name),
			ram (env, md_alloc, this->ep, name, use_inc_ckpt, granularity),
			rom (env, name)
		{
			// Donate ram quota to child
			// TODO Replace static quota donation with the amount of quota, the child needs
			Genode::size_t donate_quota = 1024*1024;
			ram.ref_account(env.ram_session_cap());
			// Note: transfer goes directly to parent's ram session
			env.ram().transfer_quota(ram.parent_cap(), donate_quota);
		}

		/**
		 * Destructor
		 */
		~Resources() { }
	} _resources;

	/**
	 * Needed for child's creation
	 */
	Genode::Child::Initial_thread  _initial_thread;
	/**
	 * Needed for child's creation
	 */
	Genode::Region_map_client      _address_space;
	/**
	 * Registry for parent's services (parent of rtcr component). It is shared between all children.
	 */
	Genode::Service_registry      &_parent_services;
	/**
	 * Registry for local services.
	 *
	 * TODO Are there any local services provided to the children?
	 */
	Genode::Service_registry       _local_services;
	/**
	 * Registry for announced services from this child
	 */
	Genode::Service_registry       _child_services;
	/**
	 * Chlid object
	 */
	Genode::Child                  _child;

public:

	/**
	 * Constructor
	 *
	 * \param env             Local environment
	 * \param md_alloc        Local allocator for child's resources and for storing services
	 * \param parent_services Registry for services provided by parent. It is shared by all children.
	 * \param name            Child's unique name and filename of child's rom module
	 * \param granularity     Granularity for incremental checkpointing in multiple of pagesize
	 *
	 * TODO Separate child's name and filename to support multiple child's with the same rom module
	 */
	Target_child(Genode::Env &env, Genode::Allocator &md_alloc,
			Genode::Service_registry &parent_services, const char *name,
			bool use_inc_ckpt = true, Genode::size_t granularity = 1)
	:
		_name            (name),
		_env             (env),
		_md_alloc        (md_alloc),
		_use_inc_ckpt    (use_inc_ckpt),
		_resources_ep    (_env, 16*1024, "resources ep"),
		_child_ep        (_env, 16*1024, "child ep"),
		_granularity     (granularity),
		_resources       (_env, _resources_ep, _md_alloc, _name.string(), _use_inc_ckpt, _granularity),
		_initial_thread  (_resources.cpu, _resources.pd.cap(), _name.string()),
		_address_space   (_resources.pd.address_space()),
		_parent_services (parent_services),
		_local_services  (),
		_child_services  (),
		_child(_resources.rom.dataspace(), Genode::Dataspace_capability(),
				_resources.pd.cap(),  _resources.pd,
				_resources.ram.cap(), _resources.ram,
				_resources.cpu.cap(), _initial_thread,
				_env.rm(), _address_space, _child_ep.rpc_ep(), *this)
	{
		if(verbose_debug) Genode::log("Target_child \"", _name.string(), "\" created.");
	}

	/**
	 * Return the custom Pd session
	 */
	Rtcr::Pd_session_component  &pd()  { return _resources.pd;  }
	/**
	 * Return the custom Cpu session
	 */
	Rtcr::Cpu_session_component &cpu() { return _resources.cpu; }
	/**
	 * Return the custom Ram session
	 */
	Rtcr::Ram_session_component &ram() { return _resources.ram; }

	/**
	 * Pause child
	 */
	void pause()  { _resources.cpu.pause_threads();  }

	/**
	 * Resume child
	 */
	void resume() { _resources.cpu.resume_threads(); }


	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return _name.string(); }

	Genode::Service *resolve_session_request(const char *service_name, const char *args)
	{
		if(verbose_debug) Genode::log("Target_child::\033[33m", "resolve_session_request", "\033[0m(", service_name, " ", args, ")");

		// TODO Support grandchildren: PD, CPU, and RAM session has also to be provided to them

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

/*		if(!Genode::strcmp(service_name, "RM"))
		{
			Rm_root *root = new (_md_alloc) Rm_root(_env, _md_alloc, _resources_ep);
			service = new (_md_alloc) Genode::Local_service(service_name, root);
			_local_services.insert(service);
			if(verbose_debug) Genode::log("  inserted service into local_services");
		}
*/
/*
		// XXX Service is LOG or Timer (temporary solution until these services are also intercepted by Rtcr)
		if(Genode::strcmp(service_name, "LOG") || Genode::strcmp(service_name, "Timer"))
		{
			service = new (_md_alloc) Genode::Parent_service(service_name);
			_parent_services.insert(service);
		}
*/
		// Service not known: Assume parent provides it; fill parent service registry on demand
		if(!service)
		{
			// TODO Store allocated objects for a later deallocate, when the child is removed
/*
			// Find root object
			Genode::Root *root = nullptr;
			if(Genode::strcmp(service_name, "Rm_session"))
			{
				root = new (_md_alloc) Rm_root(_env, _md_alloc, _resources_ep);
			}

			service = new (_md_alloc) Genode::Local_service(service_name, root);
			_local_services.insert(service);

			if(verbose_debug) Genode::log("  inserted service into local_services");
*/

			service = new (_md_alloc) Genode::Parent_service(service_name);
			_parent_services.insert(service);
			if(verbose_debug) Genode::log("  inserted service into parent_services");
		}

		return service;
	}

	void filter_session_args(const char *service, char *args, Genode::size_t args_len)
	{
		Genode::Arg_string::set_arg_string(args, args_len, "label", _name.string());
	}
};

#endif /* _RTCR_TARGET_CHILD_H_ */
