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

#include "intercept/cpu_session.h"
/* Rtcr includes */
#include "intercept/rm_session.h"
#include "intercept/log_session.h"
#include "intercept/pd_session.h"
#include "intercept/ram_session.h"
#include "intercept/rom_session.h"
#include "intercept/timer_session.h"
#include "target_state.h"

namespace Rtcr {
	class Target_child;

	constexpr bool child_verbose_debug = true;

	// Forward declaration
	class Restorer;
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
	 * Restorer needed for restoring a child
	 */
	Restorer           *_restorer;
	/**
	 * Indicator whether child was bootstraped or not
	 */
	bool                _in_bootstrap;
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
		 * Custom PD RPC object
		 */
		Pd_session_component   pd;
		/**
		 * Custom CPU RPC object
		 */
		Cpu_session_component  cpu;
		/**
		 * Custom RAM RPC object
		 */
		Ram_session_component  ram;
		/**
		 * Custom ROM RPC object
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
	 * Registry for parent's services (parent of RTCR component). It is shared between all children.
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
	 * Root RPC object of custom RM session
	 */
	Rm_root                       *_rm_root;
	/**
	 * Root RPC object of custom LOG session
	 */
	Log_root                      *_log_root;
	/**
	 * Root RPC object of custom Timer session
	 */
	Timer_root                    *_timer_root;
	/**
	 * Child object
	 */
	Genode::Child                 *_child;

	friend class Restorer;

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
	Pd_session_component  &pd()  { return _resources.pd;  }
	/**
	 * Return the custom Cpu session
	 */
	Cpu_session_component &cpu() { return _resources.cpu; }
	/**
	 * Return the custom Ram session
	 */
	Ram_session_component &ram() { return _resources.ram; }
	/**
	 * Return custom RM root
	 */
	Rm_root* rm_root()           { return _rm_root;       }
	/**
	 * Return custom LOG root
	 */
	Log_root* log_root()         { return _log_root;      }
	/**
	 * Return custom Timer root
	 */
	Timer_root* timer_root()     { return _timer_root;    }
	/**
	 * Start child from scratch
	 */
	void start();
	/**
	 * Start child from a checkpointed state
	 */
	void start(Restorer &restorer);
	/**
	 * Pause child
	 */
	void pause()  { _resources.cpu.pause_threads();  }
	/**
	 * Resume child
	 */
	void resume() { _resources.cpu.resume_threads(); }
	/**
	 * Print method to use it with Genode::log()
	 */
	void print(Genode::Output &output) const;

	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return _name.string(); }
	Genode::Service *resolve_session_request(const char *service_name, const char *args);
	void filter_session_args(const char *service, char *args, Genode::size_t args_len);

};

#endif /* _RTCR_TARGET_CHILD_H_ */
