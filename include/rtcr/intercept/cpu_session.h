/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \date   2016-08-10
 */

#ifndef _RTCR_CPU_SESSION_H_
#define _RTCR_CPU_SESSION_H_

/* Genode includes */
#include <root/component.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <cpu_session/connection.h>
#include <cpu_thread/client.h>

/* Rtcr includes */
#include "../online_storage/cpu_session_info.h"
#include "cpu_thread_component.h"
#include "pd_session.h"

namespace Rtcr {
	class Cpu_session_component;
	class Cpu_root;

	constexpr bool cpu_verbose_debug = true;
	constexpr bool cpu_root_verbose_debug = false;
}


/**
 * This custom Cpu session intercepts the creation and destruction of threads by the client
 */
class Rtcr::Cpu_session_component : public Genode::Rpc_object<Genode::Cpu_session>,
                                    private Genode::List<Cpu_session_component>::Element
{
private:
	friend class Genode::List<Rtcr::Cpu_session_component>;
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = cpu_verbose_debug;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env        &_env;
	/**
	 * Allocator for objects belonging to the monitoring of threads (e.g. Thread_info)
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Entrypoint
	 */
	Genode::Entrypoint &_ep;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool               &_bootstrap_phase;
	/**
	 * PD root for the list of all PD sessions known to child
	 *
	 * Is used to translate child's known PD session (= custom PD session) to parent's PD session.
	 */
	Pd_root            &_pd_root;
	/**
	 * Connection to parent's Cpu session, usually from core; this class wraps this session
	 */
	Genode::Cpu_connection _parent_cpu;
	/**
	 * State of parent's RPC object
	 */
	Cpu_session_info       _parent_state;

	Cpu_thread_component &_create_thread(Genode::Pd_session_capability child_pd_cap, Genode::Pd_session_capability parent_pd_cap,
			Name const &name, Genode::Affinity::Location affinity, Weight weight, Genode::addr_t utcb);
	void _kill_thread(Cpu_thread_component &cpu_thread);

	/*
	 * KIA4SM method
	 */
	Cpu_thread_component &_create_fp_edf_thread(Genode::Pd_session_capability child_pd_cap, Genode::Pd_session_capability parent_pd_cap,
			Name const &name, Genode::Affinity::Location affinity, Weight weight, Genode::addr_t utcb, unsigned priority, unsigned deadline);

public:
	Cpu_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
			Pd_root &pd_root, const char *label, const char *creation_args, bool &bootstrap_phase);
	~Cpu_session_component();

	Genode::Cpu_session_capability parent_cap() { return _parent_cpu.cap(); }

	Cpu_session_info &parent_state() { return _parent_state; }
	Cpu_session_info const &parent_state() const { return _parent_state; }

	Cpu_session_component *find_by_badge(Genode::uint16_t badge);

	using Genode::List<Rtcr::Cpu_session_component>::Element::next;

	/***************************
	 ** Cpu_session interface **
	 ***************************/

	Genode::Thread_capability create_thread(Genode::Pd_session_capability pd_cap,
			Name const &name, Genode::Affinity::Location affinity, Weight weight,
			Genode::addr_t utcb) override;
	void kill_thread(Genode::Thread_capability thread_cap) override;

	void exception_sigh(Genode::Signal_context_capability handler) override;

	Genode::Affinity::Space affinity_space() const override;
	Genode::Dataspace_capability trace_control() override;
	Quota quota() override;
	int ref_account(Genode::Cpu_session_capability c) override;
	int transfer_quota(Genode::Cpu_session_capability c, Genode::size_t q) override;
	Genode::Capability<Native_cpu> native_cpu() override;

	void deploy_queue(Genode::Dataspace_capability ds) override;
	void rq(Genode::Dataspace_capability ds) override;
	void dead(Genode::Dataspace_capability ds) override;
	void killed() override;
};


/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Cpu_root : public Genode::Root_component<Cpu_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = cpu_root_verbose_debug;

	/**
	 * Environment of Rtcr; is forwarded to a created session object
	 */
	Genode::Env        &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint &_ep;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool               &_bootstrap_phase;
	/**
	 * Monitor's PD root for the list of all PD sessions known to the child
	 *
	 * The PD sessions are used to translate child's PD sessions to parent's PD sessions.
	 * For creating a CPU thread, child needs to pass a PD session capability. Because the
	 * custom CPU session uses parent's CPU session (e.g. core's CPU session), it also has
	 * to pass a PD session which is known by the parent.
	 */
	Pd_root            &_pd_root;
	/**
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Cpu_session_component> _session_rpc_objs;

protected:
	Cpu_session_component *_create_session(const char *args);
	void _upgrade_session(Cpu_session_component *session, const char *upgrade_args);
	void _destroy_session(Cpu_session_component *session);

public:
	Cpu_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep,
			Pd_root &pd_root, bool &bootstrap_phase);
    ~Cpu_root();

	Genode::List<Cpu_session_component> &session_infos() { return _session_rpc_objs; }
};

#endif /* _RTCR_CPU_SESSION_H_ */
