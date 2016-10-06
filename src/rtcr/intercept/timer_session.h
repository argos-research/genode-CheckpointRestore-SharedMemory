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

namespace Rtcr {
	struct Timer_session_info;
	class Timer_session_component;
	class Timer_root;

	constexpr bool timer_verbose_debug = false;
	constexpr bool timer_root_verbose_debug = false;
}


/**
 * List element for monitoring session objects.
 * Each new connection from client to server is monitored here.
 */
struct Rtcr::Timer_session_info : Genode::List<Timer_session_info>::Element
{
	/**
	 * Reference to the session object; encapsulates capability and object's state
	 */
	Timer_session_component &session;
	/**
	 * Arguments provided for creating the session object
	 */
	const char *args;

	Timer_session_info(Timer_session_component &comp, const char* args)
	:
		session(comp),
		args(args)
	{ }

	Timer_session_info *find_by_ptr(Timer_session_component *ptr)
	{
		if(ptr == &session)
			return this;
		Timer_session_info *info = next();
		return info ? info->find_by_ptr(ptr) : 0;
	}
};

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
	Timer_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *args)
	:
		_md_alloc     (md_alloc),
		_ep           (ep),
		_parent_timer (env)
	{
		if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
	}

	~Timer_session_component() 
    { 
        if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
    }

	/************************************
	 ** Timer session Rpc interface **
	 ************************************/

	void trigger_once(unsigned us)
	{
		if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(us=", us, ")");
		_parent_state.timeout = us;
		_parent_state.periodic = false;
		_parent_timer.trigger_once(us);
	}

	void trigger_periodic(unsigned us)
	{
		if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(us=", us, ")");
		_parent_state.timeout = us;
		_parent_state.periodic = true;
		_parent_timer.trigger_periodic(us);
	}

	void sigh(Genode::Signal_context_capability sigh)
	{
		if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(", sigh, ")");

		_parent_state.sigh = sigh;

		_parent_timer.sigh(sigh);
	}

	unsigned long elapsed_ms() const
	{
		if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m()");
		auto result = _parent_timer.elapsed_ms();
		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	void msleep(unsigned ms)
	{
		if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(ms=", ms, ")");
		_parent_state.timeout = 1000*ms;
		_parent_state.periodic = false;
		_parent_timer.msleep(ms);
	}

	void usleep(unsigned us)
	{
		if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(us=", us, ")");
		_parent_state.timeout = us;
		_parent_state.periodic = false;
		_parent_timer.usleep(us);
	}
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
	Timer_session_component *_create_session(const char *args)
	{
		if(verbose_debug) Genode::log("Timer_root::\033[33m", __func__, "\033[0m(", args,")");

		// Create virtual session object
		Timer_session_component *new_session =
				new (md_alloc()) Timer_session_component(_env, _md_alloc, _ep, args);

		// Create and insert list element
		Timer_session_info *new_session_info =
				new (md_alloc()) Timer_session_info(*new_session, args);
		Genode::Lock::Guard guard(_infos_lock);
		_session_infos.insert(new_session_info);

		return new_session;
	}

	/**
	 * Destroy session object and its monitoring list element
	 */
	void _destroy_session(Timer_session_component *session)
	{
		if(verbose_debug) Genode::log("Timer_root::\033[33m", __func__, "\033[0m(ptr=", session,")");
		// Find and destroy list element and its session object
		Timer_session_info *info = _session_infos.first();
		if(info) info = info->find_by_ptr(session);
		if(info)
		{
			// Remove and destroy list element
			_session_infos.remove(info);
			destroy(_md_alloc, info);

			// Destroy virtual session object
			destroy(_md_alloc, session);
		}
		// No list element found
		else
		{
			Genode::error("Timer_root: Session not found in the list");
		}
	}

public:
	Timer_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep)
	:
		Root_component<Timer_session_component>(session_ep, md_alloc),
		_env           (env),
		_md_alloc      (md_alloc),
		_ep            (session_ep),
		_infos_lock    (),
		_session_infos ()
	{
		if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
	}
    
    ~Timer_root()
    {
        Timer_session_info *info = nullptr;
        
        while((info = _session_infos.first()))
        {
            _session_infos.remove(info);
            Genode::destroy(_md_alloc, info);
        }
        
        if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
    }
    
    Genode::List<Timer_session_info> &session_infos() { return _session_infos;  }
    void session_infos(Genode::List<Timer_session_info> &infos) { _session_infos = infos; }
};

#endif /* _RTCR_TIMER_SESSION_H_ */
