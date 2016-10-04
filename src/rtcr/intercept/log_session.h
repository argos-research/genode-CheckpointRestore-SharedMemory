/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \date   2016-10-02
 */

#ifndef _RTCR_LOG_SESSION_H_
#define _RTCR_LOG_SESSION_H_

/* Genode includes */
#include <log_session/connection.h>
#include <root/component.h>
#include <util/list.h>

namespace Rtcr {
	struct Log_session_info;
	class Log_session_component;
	class Log_root;

	constexpr bool log_verbose_debug = false;
	constexpr bool log_root_verbose_debug = false;
}

/**
 * List element for monitoring session objects.
 * Each new connection from client to server is monitored here.
 */
struct Rtcr::Log_session_info : Genode::List<Log_session_info>::Element
{
	/**
	 * Reference to session object,
	 * encapsulates capability which is the main reason for storing it
	 */
	Log_session_component &session;
	/**
	 * Arguments provided for creating the session object
	 */
	const char            *args;

	Log_session_info(Log_session_component &comp, const char* args)
	:
		session(comp),
		args(args)
	{ }

	Log_session_info *find_by_ptr(Log_session_component *ptr)
	{
		if(ptr == &session)
			return this;
		Log_session_info *info = next();
		return info ? info->find_by_ptr(ptr) : 0;
	}
};

/**
 * Virtual session object to intercept Rpc object creation and
 * state modifications of the wrapped, parent session object
 */
class Rtcr::Log_session_component : public Genode::Rpc_object<Genode::Log_session>
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

public:
	Log_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *args)
	:
		_md_alloc   (md_alloc),
		_ep         (ep),
		_parent_log (env, args)
	{
		if(verbose_debug) Genode::log("\033[33m", "Log_session_component", "\033[0m created");
	}

	~Log_session_component() { }

	/*******************************
	 ** Log session Rpc interface **
	 *******************************/

	Genode::size_t write(String const &string)
	{
		if(verbose_debug) Genode::log("Log::\033[33m", "write", "\033[0m(", string.string(),")");
		auto result = _parent_log.write(string);
		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}


};

/**
 * Virtual Root session object to intercept session object creation
 * This enables the Rtcr component to monitor capabilities created for session objects
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
	Genode::Lock                    _infos_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Log_session_info>  _session_infos;

protected:
	/**
	 * Create session object and its monitoring list element
	 */
	Log_session_component *_create_session(const char *args)
	{
		Genode::log("Log_root::\033[33m", "_create_session", "\033[0m(", args,")");

		// Extracting label from args
		char label_buf[128];
		Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
		label_arg.string(label_buf, sizeof(label_buf), "");

		// Create virtual session object
		Log_session_component *new_session =
				new (md_alloc()) Log_session_component(_env, _md_alloc, _ep, label_buf);

		// Create and insert list element
		Log_session_info *new_session_info =
				new (md_alloc()) Log_session_info(*new_session, args);
		Genode::Lock::Guard guard(_infos_lock);
		_session_infos.insert(new_session_info);

		return new_session;
	}

	/**
	 * Destroy session object and its monitoring list element
	 */
	void _destroy_session(Log_session_component *session)
	{
		if(verbose_debug) Genode::log("Log_root::\033[33m", "_destroy_session", "\033[0m(ptr=", session,")");
		// Find and destroy list element and its session object
		Log_session_info *info = _session_infos.first();
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
			Genode::error("Log_root: Session not found in the list");
		}
	}

public:
	Log_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep)
	:
		Root_component<Log_session_component>(session_ep, md_alloc),
		_env           (env),
		_md_alloc      (md_alloc),
		_ep            (session_ep),
		_infos_lock    (),
		_session_infos ()
	{
		if(verbose_debug) Genode::log("\033[33m", "Log_root", "\033[0m created");
	}
};

#endif /* _RTCR_LOG_SESSION_H_ */
