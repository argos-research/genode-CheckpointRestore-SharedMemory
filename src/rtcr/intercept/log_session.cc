/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \date   2016-10-02
 */

#include "log_session.h"

using namespace Rtcr;


Log_session_component::Log_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *args)
:
	_md_alloc   (md_alloc),
	_ep         (ep),
	_parent_log (env, args)
{
	if(verbose_debug) Genode::log("\033[33m", "Log", "\033[0m(parent ", _parent_log,")");
}


Log_session_component::~Log_session_component()
{
	if(verbose_debug) Genode::log("\033[33m", "~Log", "\033[0m ", _parent_log);
}


Genode::size_t Log_session_component::write(String const &string)
{
	if(verbose_debug) Genode::log("Log::\033[33m", __func__, "\033[0m(", string.string(),")");
	auto result = _parent_log.write(string);
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Log_session_component *Log_root::_create_session(const char *args)
{
	if(verbose_debug) Genode::log("Log_root::\033[33m", __func__, "\033[0m(", args,")");

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


void Log_root::_destroy_session(Log_session_component *session)
{
	if(verbose_debug) Genode::log("Log_root::\033[33m", __func__, "\033[0m(ptr=", session,")");
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


Log_root::Log_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep)
:
	Root_component<Log_session_component>(session_ep, md_alloc),
	_env           (env),
	_md_alloc      (md_alloc),
	_ep            (session_ep),
	_infos_lock    (),
	_session_infos ()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Log_root::~Log_root()
{
    Log_session_info *info = nullptr;

    while((info = _session_infos.first()))
    {
        _session_infos.remove(info);
        Genode::destroy(_md_alloc, info);
    }

    if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}
