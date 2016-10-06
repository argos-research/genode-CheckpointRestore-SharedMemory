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
#include "intercept/log_session.h"
#include "intercept/timer_session.h"

namespace Rtcr {
	class Target_child;

	constexpr bool child_verbose_debug = true;

	class Pd_session_component;
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
		Genode::Entrypoint    &ep;
		/**
		 * Custom Pd Rpc_object
		 */
		Pd_session_component   pd;
		/**
		 * Custom Cpu Rpc_object
		 */
		Cpu_session_component  cpu;
		/**
		 * Custom Ram Rpc_object
		 */
		Ram_session_component  ram;
		/**
		 * Parent's Rom session (usually from core)
		 */
		Genode::Rom_connection rom;

		/**
		 * Constructor
		 */
		Resources(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc, const char *name,
				bool use_inc_ckpt = true, Genode::size_t granularity = 1);
		/**
		 * Destructor
		 */
		~Resources();
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
	 * TODO Separate child's name and filename to support multiple child's with the same rom module
	 */
	Target_child(Genode::Env &env, Genode::Allocator &md_alloc,
			Genode::Service_registry &parent_services, const char *name,
			bool use_inc_ckpt = true, Genode::size_t granularity = 1);

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

	Genode::Service *resolve_session_request(const char *service_name, const char *args);

	void filter_session_args(const char *service, char *args, Genode::size_t args_len);
};

#endif /* _RTCR_TARGET_CHILD_H_ */
