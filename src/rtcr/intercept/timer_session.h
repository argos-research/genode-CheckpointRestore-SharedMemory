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
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/timer_session_info.h"

namespace Rtcr {
	class Timer_session_component;
	class Timer_root;

	constexpr bool timer_verbose_debug = false;
	constexpr bool timer_root_verbose_debug = false;
}


/**
 * Virtual session object to intercept Rpc object creation and
 * state modifications of the wrapped, parent session object
 */
class Rtcr::Timer_session_component : public Genode::Rpc_object<Timer::Session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = timer_verbose_debug;
	/**
	 * Allocator for Rpc objects created by this session and also for monitoring structures
	 */
	Genode::Allocator             &_md_alloc;
	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint            &_ep;
	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Timer::Connection              _parent_timer;
	/**
	 * Parent's session state
	 */
	struct State_info
	{
		Genode::Signal_context_capability sigh     {};
		unsigned                          timeout  {};
		bool                              periodic {};
	} _parent_state;

public:
	Timer_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *args);
	~Timer_session_component();

	/************************************
	 ** Timer session Rpc interface **
	 ************************************/

	void trigger_once(unsigned us) override;
	void trigger_periodic(unsigned us) override;
	void sigh(Genode::Signal_context_capability sigh) override;
	unsigned long elapsed_ms() const override;
	void msleep(unsigned ms) override;
	void usleep(unsigned us) override;
};

/**
 * Virtual Root session object to intercept session object creation
 * This enables the Rtcr component to monitor capabilities created for session objects
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
	Genode::Env                    &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator              &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint             &_ep;
	/**
	 * Lock for infos list
	 */
	Genode::Lock                      _infos_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Timer_session_info>  _session_infos;

protected:
	/**
	 * Create session object and its monitoring list element
	 */
	Timer_session_component *_create_session(const char *args);
	/**
	 * Destroy session object and its monitoring list element
	 */
	void _destroy_session(Timer_session_component *session);

public:
	Timer_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep);
    ~Timer_root();
    
    Genode::List<Timer_session_info> &session_infos()           { return _session_infos;  }
    void session_infos(Genode::List<Timer_session_info> &infos) { _session_infos = infos; }
};

#endif /* _RTCR_TIMER_SESSION_H_ */
