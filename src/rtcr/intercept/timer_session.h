/*
 * \brief  Intercepting timer session
 * \author Denis Huber
 * \date   2016-10-05
 */

#ifndef _RTCR_TIMER_SESSION_H_
#define _RTCR_TIMER_SESSION_H_

/* Genode includes */
#include <timer_session/connection.h>
#include <root/component.h>
#include <base/allocator.h>
#include <util/list.h>

/* Rtcr includes */
#include "../online_storage/timer_session_info.h"

namespace Rtcr {
	class Timer_session_component;
	class Timer_root;

	constexpr bool timer_verbose_debug = false;
	constexpr bool timer_root_verbose_debug = false;
}

/**
 * Custom RPC session object to intercept its creation, modification, and destruction through its interface
 */
class Rtcr::Timer_session_component : public Genode::Rpc_object<Timer::Session>,
                                      public Genode::List<Timer_session_component>::Element
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = timer_verbose_debug;
	/**
	 * Allocator for Rpc objects created by this session and also for monitoring structures
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint &_ep;
	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Timer::Connection   _parent_timer;
	/**
	 * State of parent's RPC object
	 */
	Timer_session_info  _parent_state;

public:
	Timer_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
			const char *creation_args, bool bootstrapped = false);
	~Timer_session_component();

	Timer::Session_capability parent_cap() { return _parent_timer.cap(); }

	Timer_session_info &parent_state() { return _parent_state; }
	Timer_session_info const &parent_state() const { return _parent_state; }

	Timer_session_component *find_by_badge(Genode::uint16_t badge);

	/************************************
	 ** Timer session Rpc interface **
	 ************************************/

	void trigger_once(unsigned us) override;
	void trigger_periodic(unsigned us) override;
	void sigh(Genode::Signal_context_capability sigh) override;
	unsigned long elapsed_ms() const override;
	void msleep(unsigned ms) override;
	void usleep(unsigned us) override;
	unsigned long now_us() const override;
};

/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Timer_root : public Genode::Root_component<Timer_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = timer_root_verbose_debug;

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
	 * List for monitoring session objects
	 */
	Genode::List<Timer_session_component> _session_rpc_objs;

protected:
	Timer_session_component *_create_session(const char *args);
	void _upgrade_session(Timer_session_component *session, const char *upgrade_args);
	void _destroy_session(Timer_session_component *session);

public:
	Timer_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep, bool &bootstrap_phase);
    ~Timer_root();
    
    Genode::List<Timer_session_component> &session_infos() { return _session_rpc_objs;  }
};

#endif /* _RTCR_TIMER_SESSION_H_ */
