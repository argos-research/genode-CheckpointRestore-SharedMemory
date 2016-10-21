/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \date   2016-08-10
 */

#ifndef _RTCR_CPU_SESSION_COMPONENT_H_
#define _RTCR_CPU_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <cpu_session/connection.h>
#include <cpu_thread/client.h>

/* Rtcr includes */
#include "../monitor/thread_info.h"
#include "cpu_thread_component.h"

namespace Rtcr {
	class Cpu_session_component;

	constexpr bool cpu_session_verbose_debug = false;
}


/**
 * This custom Cpu session intercepts the creation and destruction of threads by the client
 */
class Rtcr::Cpu_session_component : public Genode::Rpc_object<Genode::Cpu_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = cpu_session_verbose_debug;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env                   &_env;
	/**
	 * Allocator for objects belonging to the monitoring of threads (e.g. Thread_info)
	 */
	Genode::Allocator             &_md_alloc;
	/**
	 * Entrypoint to manage itself
	 */
	Genode::Entrypoint            &_ep;
	/**
	 * Parent Pd session, usually from core; used for creating a thread
	 */
	Genode::Pd_session_capability  _parent_pd_cap;
	/**
	 * Connection to parent's Cpu session, usually from core; this class wraps this session
	 */
	Genode::Cpu_connection         _parent_cpu;
	/**
	 * Parent's session state
	 */
	struct State_info
	{
		Genode::Signal_context_capability exception_sigh {};
	} _parent_state;
	/**
	 * Lock to make _threads thread-safe
	 */
	Genode::Lock                   _threads_lock;
	/**
	 * List of client's thread capabilities
	 */
	Genode::List<Thread_info>      _threads;

	/**
	 * Removes info from Thread_info list, destroys the containing virtual RPC object,
	 * and destroys the info itself
	 */
	void _destroy(Thread_info* info);

public:

	/**
	 * Constructor
	 */
	Cpu_session_component(Genode::Env &env, Genode::Allocator &md_alloc,
			Genode::Entrypoint &ep, Genode::Pd_session_capability parent_pd_cap,
			const char *name);

	/**
	 * Destructor
	 */
	~Cpu_session_component();

	Genode::Cpu_session_capability  parent_cap()   { return _parent_cpu.cap(); }
	Genode::List<Thread_info>      &thread_infos() { return _threads; }

	/**
	 * Pause all threads
	 */
	void pause_threads();

	/**
	 * Resume all threads
	 */
	void resume_threads();

	/***************************
	 ** Cpu_session interface **
	 ***************************/

	/**
	 * Create a thread and store its capability in Thread_info
	 */
	Genode::Thread_capability create_thread(Genode::Pd_session_capability /* pd_cap */,
			Name const &name, Genode::Affinity::Location affinity, Weight weight,
			Genode::addr_t utcb) override;
	/**
	 * Destroy thread and its Thread_info
	 */
	void kill_thread(Genode::Thread_capability thread) override;
	void exception_sigh(Genode::Signal_context_capability handler) override;
	Genode::Affinity::Space affinity_space() const override;
	Genode::Dataspace_capability trace_control() override;
	Quota quota() override;
	int ref_account(Genode::Cpu_session_capability c) override;
	int transfer_quota(Genode::Cpu_session_capability c, Genode::size_t q) override;
	Genode::Capability<Native_cpu> native_cpu() override;
};

#endif /* _RTCR_CPU_SESSION_COMPONENT_H_ */
