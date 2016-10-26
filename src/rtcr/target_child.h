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
#include <base/service.h>
#include <rom_session/client.h>

/* Rtcr includes */
#include "intercept/cpu_session_component.h"
#include "intercept/pd_session_component.h"
#include "intercept/ram_session_component.h"
#include "intercept/rom_session_component.h"
#include "intercept/rm_session.h"
#include "intercept/log_session.h"
#include "intercept/timer_session.h"
#include "target_state.h"

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
	 * Entrypoint for managing child's resource-sessions (PD, CPU, RAM)
	 */
	Genode::Entrypoint  _resources_ep;
	/**
	 * Entrypoint for child's creation
	 */
	Genode::Entrypoint  _child_ep;
	/**
	 * Granularity for incremental checkpointing in a multiple of pagesize;
	 * zero means do not use incremental checkpointing
	 */
	Genode::size_t      _granularity;
	/**
	 * Indicates whether the start of new threads shall be postponed
	 */
	bool                _phase_restore;
	/**
	 * Pointer to a checkpointed state of the child
	 */
	Target_state       *_state_restore;
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
		Rom_session_component  rom;

		/**
		 * Constructor
		 */
		Resources(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc,
				const char *name, Genode::size_t granularity);
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
	 * Chlid object in heap
	 */
	Genode::Child                 *_child;

	/**
	 * Restore state of the child
	 */
	void _restore();

public:

	/**
	 * Constructor
	 *
	 * TODO Separate child's name and filename to support multiple child's with the same rom module
	 */
	Target_child(Genode::Env &env, Genode::Allocator &md_alloc,
			Genode::Service_registry &parent_services, const char *name,
			Genode::size_t granularity = 1);

	~Target_child();

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
	 * Start child by creating a Genode::Child object
	 */
	void start();
	/**
	 * Start child with a checkpointed state
	 */
	void start(Target_state &state);
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
