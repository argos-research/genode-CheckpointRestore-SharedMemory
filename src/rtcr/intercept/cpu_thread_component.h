/*
 * \brief  Intercepting Cpu thread
 * \author Denis Huber
 * \date   2016-10-21
 */

#ifndef _RTCR_CPU_THREAD_COMPONENT_H_
#define _RTCR_CPU_THREAD_COMPONENT_H_

#include <cpu_thread/client.h>
#include <base/rpc_server.h>
#include <base/entrypoint.h>

namespace Rtcr {
	class Cpu_thread_component;

	constexpr bool cpu_thread_verbose_debug = true;
}

class Rtcr::Cpu_thread_component : public Genode::Rpc_object<Genode::Cpu_thread>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = cpu_thread_verbose_debug;

	/**
	 * Entrypoint which manages this object
	 */
	Genode::Entrypoint                 &_ep;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Cpu_thread_client           _parent_cpu_thread;
	/**
	 * Indicator for a restore phase: The thread shall not be started immediately, but after the address space is restored
	 */
	bool                               &_phase_restore;
	/**
	 * Name of the thread
	 */
	Genode::Cpu_session::Name           _name;
	/**
	 * Parent's session state
	 */
	struct State_info
	{
		State_info();
		bool                              started;
		Genode::addr_t                    ip;
		Genode::addr_t                    sp;
		bool                              paused;
		Genode::Signal_context_capability exception_sigh;
		bool                              single_step;
		Genode::Affinity::Location        location;
	} _parent_state;

public:

	Cpu_thread_component(Genode::Entrypoint &ep, Genode::Capability<Genode::Cpu_thread> cpu_th_cap,
			bool &phase_restore, Genode::Cpu_session::Name name);
	~Cpu_thread_component();

	Genode::Capability<Genode::Cpu_thread> parent_cap()   { return _parent_cpu_thread; }
	State_info                             parent_state() { return _parent_state; }

	/******************************
	 ** Cpu thread Rpc interface **
	 ******************************/

	Genode::Dataspace_capability utcb                () override;
	void                         start               (Genode::addr_t ip, Genode::addr_t sp) override;
	void                         pause               () override;
	void                         resume              () override;
	void                         cancel_blocking     () override;
	Genode::Thread_state         state               () override;
	void                         state               (Genode::Thread_state const &state) override;
	void                         exception_sigh      (Genode::Signal_context_capability handler) override;
	void                         single_step         (bool enabled) override;
	void                         affinity            (Genode::Affinity::Location location) override;
	unsigned                     trace_control_index () override;
	Genode::Dataspace_capability trace_buffer        () override;
	Genode::Dataspace_capability trace_policy        () override;
};

#endif /* _RTCR_CPU_THREAD_COMPONENT_H_ */
