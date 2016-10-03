/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \date   2016-10-02
 */

#ifndef _RTCR_LOG_SESSION_H_
#define _RTCR_LOG_SESSION_H_

/* Genode includes */
#include <log_session/log_session.h>
#include <root/component.h>
#include <util/list.h>

namespace Rtcr {
	struct Log_session_info;
	class Log_session_component;
	class Log_root;

	constexpr bool log_verbose_debug = true;
	constexpr bool log_root_verbose_debug = true;
}

struct Rtcr::Log_session_info : Genode::List<Log_session_info>::Element
{
	Log_session_component &comp;
	const char *args;

	Log_session_info(Log_session_component &comp, const char* args)
	:
		comp(comp),
		args(args)
	{ }

	Log_session_info *find_by_ptr(Log_session_component *ptr)
	{
		if(ptr == &comp)
			return this;
		Log_session_info *info = next();
		return info ? info->find_by_ptr(ptr) : 0;
	}
};

class Rtcr::Log_session_component : public Genode::Rpc_object<Genode::Log_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rm_verbose_debug;
	Genode::Allocator             &_md_alloc;
	Genode::Entrypoint            &_ep;
	Genode::Log_connection         _parent_log;

public:
	Log_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *args)
	:
		_md_alloc(md_alloc),
		_ep(ep),
		_parent_log(env, args)
	{
		if(verbose_debug) Genode::log("\033[33m", "Log_session_component", "\033[0m created");
	}

	~Log_session_component() { }

	/*******************************
	 ** Log session Rpc interface **
	 *******************************/

	Genode::size_t write(String const &string);


};

class Rtcr::Log_root : public Genode::Root_component<Log_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rm_root_verbose_debug;

	Genode::Env                    &_env;
	Genode::Allocator              &_md_alloc;
	Genode::Entrypoint             &_ep;
	Genode::Lock                    _infos_lock;
	Genode::List<Log_session_info>  _session_infos;
protected:
	Log_session_component *_create_session(const char *args)
	{
		Genode::log("Log_root::\033[33m", "_create_session", "\033[0m(", args,")");

		// Create virtual session object
		Log_session_component *new_session =
				new (md_alloc()) Log_session_component(_env, _md_alloc, _ep, args);

		// Create and insert list element
		Log_session_info *new_session_info =
				new (md_alloc()) Log_session_info(*new_session, args);
		Genode::Lock::Guard guard(_infos_lock);
		_session_infos.insert(new_session_info);

		return new_session;
	}

	// TODO copy from rm_session.h

public:
};

#endif /* _RTCR_LOG_SESSION_H_ */
