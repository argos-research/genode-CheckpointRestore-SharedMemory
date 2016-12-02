/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \date   2016-10-02
 */

#ifndef _RTCR_LOG_SESSION_H_
#define _RTCR_LOG_SESSION_H_

/* Genode includes */
#include <log_session/connection.h>
#include <base/allocator.h>
#include <root/component.h>
#include <util/list.h>

/* Rtcr includes */
#include "../online_storage/log_session_info.h"

namespace Rtcr {
	class Log_session_component;
	class Log_root;

	constexpr bool log_verbose_debug = false;
	constexpr bool log_root_verbose_debug = false;
}

/**
 * Custom RPC session object to intercept its creation, modification, and destruction through its interface
 */
class Rtcr::Log_session_component : public Genode::Rpc_object<Genode::Log_session>,
                                    public Genode::List<Log_session_component>::Element
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = log_verbose_debug;
	/**
	 * Allocator for Rpc objects created by this session and also for monitoring list elements
	 */
	Genode::Allocator             &_md_alloc;
	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint            &_ep;
	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Genode::Log_connection         _parent_log;
	/**
	 * State of parent's RPC object
	 */
	Log_session_info               _parent_state;


public:
	Log_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
			const char *label, const char *creation_args, bool bootstrapped = false);
	~Log_session_component();

	Genode::Log_session_capability parent_cap() { return _parent_log.cap(); }

	Log_session_info &parent_state() { return _parent_state; }
	Log_session_info const &parent_state() const { return _parent_state; }

	Log_session_component *find_by_badge(Genode::uint16_t badge);

	/*******************************
	 ** Log session Rpc interface **
	 *******************************/

	Genode::size_t write(String const &string) override;
};

/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Log_root : public Genode::Root_component<Log_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = log_root_verbose_debug;

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
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;
	/**
	 * List for monitoring session RPC objects
	 */
	Genode::List<Log_session_component> _session_rpc_objs;

protected:
	Log_session_component *_create_session(const char *args);
	void _upgrade_session(Log_session_component *session, const char *upgrade_args);
	void _destroy_session(Log_session_component *session);

public:
	Log_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep, bool &bootstrap_phase);
    ~Log_root();

	Genode::List<Log_session_component> &session_infos() { return _session_rpc_objs; }
};

#endif /* _RTCR_LOG_SESSION_H_ */
