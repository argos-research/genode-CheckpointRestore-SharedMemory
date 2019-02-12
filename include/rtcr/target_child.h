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
#include <base/snprintf.h>
#include <rom_session/connection.h>
#include <cpu_session/cpu_session.h>

/* Rtcr includes */
#include "intercept/pd_session.h"
#include "intercept/cpu_session.h"
//#include "intercept/ram_session.h"
#include "intercept/rm_session.h"
#include "intercept/log_session.h"
#include "intercept/rom_session.h"
#include "intercept/timer_session.h"
#include "target_state.h"

#include <os/session_requester.h>

namespace Rtcr {
	class Target_child;

	constexpr bool child_verbose_debug = true;

	// Forward declaration
	class Restorer;
}


template <typename T>
inline T &find_service(Genode::Registry<T> &services,
                       Genode::Service::Name const &name)
{
	T *service = nullptr;
	services.for_each([&] (T &s) {

		if (service || s.name() != name)
			return;

		service = &s;
	});

	//if (!service)
		//throw Service_denied();

	//if (service->abandoned())
		//throw Service_denied();

	return *service;
}

/*struct Genode::Local_service<Rtcr::Pd_session_component>::Factory
{
	Rtcr::Pd_session_component Genode::Local_service<Rtcr::Pd_session_component>::Factory::create(Args const &, Affinity);
			void Genode::Local_service<Rtcr::Pd_session_component>::Factory::upgrade(Rtcr::Pd_session_component &, Args const &);
			void Genode::Local_service<Rtcr::Pd_session_component>::Factory::destroy(Rtcr::Pd_session_component &);
}*/

/**
 * Encapsulates the policy and creation of the child
 */
class Rtcr::Target_child : public Genode::Child_policy
{
private:
	Target_child(Target_child const&) = default;
        Target_child& operator=(Target_child const&) = default;
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = child_verbose_debug;

	/**
	 * Indicator whether child was bootstraped or not
	 */
	bool                _in_bootstrap;
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
	 * Struct for custom / intercepted services
	 */
	struct Intercepted_parent_service : Genode::Parent_service
	{
		Genode::Signal_context_capability fault_sigh { };

		Intercepted_parent_service(Genode::Env &env, Genode::Service::Name const &name)
		: Parent_service(env, name) { }
	};

	struct Local_cpu_service : Intercepted_parent_service
	{
		Local_cpu_service(Genode::Env &env) : Intercepted_parent_service(env, "CPU") { }

		void initiate_request(Genode::Session_state &session) override
		{
			Intercepted_parent_service::initiate_request(session);

			if (session.phase != Genode::Session_state::AVAILABLE)
				return;

			Genode::Cpu_session_client cpu(Genode::reinterpret_cap_cast<Genode::Cpu_session>(session.cap));
			cpu.exception_sigh(fault_sigh);
		}
	};

	struct Local_pd_service : Intercepted_parent_service
	{
		Local_pd_service(Genode::Env &env) : Intercepted_parent_service(env, "PD") { }

		void initiate_request(Genode::Session_state &session) override
		{
			Intercepted_parent_service::initiate_request(session);

			if (session.phase != Genode::Session_state::AVAILABLE)
				return;

			Genode::Pd_session_client pd(Genode::reinterpret_cap_cast<Genode::Pd_session>(session.cap));

			Genode::Region_map_client(pd.address_space()).fault_handler(fault_sigh);
			Genode::Region_map_client(pd.stack_area())   .fault_handler(fault_sigh);
			Genode::Region_map_client(pd.linker_area())  .fault_handler(fault_sigh);
		}
	};

	Local_cpu_service           _cpu_service { _env };
	Local_pd_service            _pd_service  { _env };

	struct Custom_services
	{
	private:
		Custom_services(Custom_services const&) = default;
        	Custom_services& operator=(Custom_services const&) = default;
		Genode::Env        &_env;
		Genode::Allocator  &_md_alloc;
		Genode::Entrypoint &_resource_ep;
		bool &_bootstrap_phase;
	public:
		Genode::Session::Resources         _resources;
		Genode::Session::Diag              diag {};
		bool foo=false;
		Pd_root *pd_root = nullptr;
		Pd_session_component *pd_session = nullptr;
		Genode::Local_service<Rtcr::Pd_session_component>::Single_session_factory *pd_factory = nullptr;
		Genode::Local_service<Rtcr::Pd_session_component> *pd_service = nullptr;

		Cpu_root *cpu_root = nullptr;
		Cpu_session_component *cpu_session = nullptr;
		Genode::Local_service<Cpu_session_component>::Single_session_factory *cpu_factory = nullptr;
		Genode::Local_service<Cpu_session_component> *cpu_service = nullptr;

		/*Ram_root *ram_root  = nullptr;
		Pd_session_component *ram_session = nullptr;
		Genode::Local_service<Pd_session_component>::Single_session_factory *ram_factory = nullptr;
		Genode::Local_service<Pd_session_component> *ram_service = nullptr;*/

		Rom_root *rom_root  = nullptr;
		Rom_session_component *rom_session = nullptr;
		Genode::Local_service<Rom_session_component>::Single_session_factory *rom_factory = nullptr;
		Genode::Local_service<Rom_session_component> *rom_service = nullptr;

		Rm_root *rm_root  = nullptr;
		Rm_session_component *rm_session = nullptr;
		Genode::Local_service<Rm_session_component>::Single_session_factory *rm_factory = nullptr;
		Genode::Local_service<Rm_session_component> *rm_service = nullptr;

		Log_root *log_root  = nullptr;
		Log_session_component *log_session = nullptr;
		Genode::Local_service<Log_session_component>::Single_session_factory *log_factory = nullptr;
		Genode::Local_service<Log_session_component> *log_service = nullptr;

		Timer_root *timer_root  = nullptr;
		Timer_session_component *timer_session = nullptr;
		Genode::Local_service<Timer_session_component>::Single_session_factory *timer_factory = nullptr;
		Genode::Local_service<Timer_session_component> *timer_service = nullptr;

		Custom_services(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
				Genode::size_t granularity, bool &bootstrap_phase, Genode::Session::Resources         resources);
		~Custom_services();

		Genode::Service *find(const char *service_name);


	} _custom_services;
	/**
	 * Child's resources
	 */
	struct Resources
	{
		/**
		 * Custom PD RPC object
		 */
		Pd_session_component   &pd;
		/**
		 * Custom CPU RPC object
		 */
		Cpu_session_component  &cpu;
		/**
		 * Custom RAM RPC object
		 */
		//Ram_session_component  &ram;
		/**
		 * Parent's ROM session
		 */
		Genode::Rom_connection  rom;

		Resources(Genode::Env &env, Genode::Allocator &md_alloc, const char *label, Custom_services &custom_services);
		~Resources();

		Pd_session_component &init_pd(const char *label, Pd_root &pd_root, Genode::Allocator &_md_alloc);
		Cpu_session_component &init_cpu(const char *label, Cpu_root &cpu_root, Genode::Allocator &_md_alloc);
		//Ram_session_component &init_ram(const char *label, Ram_root &ram_root);
	} _resources;

	/**
	 * Needed for child's creation
	 */
	//Genode::Child::Initial_thread  _initial_thread;
	/**
	 * Needed for child's creation
	 */
	Genode::Region_map_client      _address_space;
	/**
	 * Registry for parent's services (parent of RTCR component). It is shared between all children.
	 */
	Genode::Registry<Genode::Registered<Genode::Parent_service> > &_parent_services;
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
			Genode::Registry<Genode::Registered<Genode::Parent_service> > &_parent_services, const char *name,
			Genode::size_t granularity);

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
	//Ram_session_component &ram() { return _resources.ram; }
	/**
	 * Return the struct of custom services
	 */
	Custom_services &custom_services() { return _custom_services; }
	/**
	 * Start child from scratch
	 */
	void start();
	/**
	 * Start child from a checkpointed state
	 */
	void start(Restorer &restorer);
	/**
	 * Pause all child's threads
	 */
	void pause();
	/**
	 * Resume all child's threads
	 */
	void resume();
	/**
	 * Print method to use it with Genode::log()
	 */
	void print(Genode::Output &output) const;

	/****************************
	 ** Child-policy interface **
	 ****************************/

	Name name() const {return _name.string(); }
	Genode::Child_policy::Route resolve_session_request(Genode::Service::Name const &,
		                              Genode::Session_label const &) override;
	//void filter_session_args(Genode::Service::Name const &,
	//                                 char * /*args*/, Genode::size_t /*args_len*/) override;
	//void init(Genode::Pd_session &, Genode::Capability<Genode::Pd_session>) override;

	Genode::Pd_session           &ref_pd() { return _resources.pd;  }
	Genode::Pd_session_capability ref_pd_cap() const { return _resources.pd.cap();  }

};

#endif /* _RTCR_TARGET_CHILD_H_ */
