/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \date   2016-10-02
 */

#include "log_session.h"

using namespace Rtcr;


Log_session_component::Log_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
		const char *args, bool bootstrapped)
:
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_log   (env, args),
	_parent_state (args, bootstrapped)
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
	if(verbose_debug) Genode::log("Log_root::\033[33m", __func__, "\033[0m(args='", args, "')");

	// Extracting label from args
	char label_buf[128];
	Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
	label_arg.string(label_buf, sizeof(label_buf), "");

	// Create virtual session object
	Log_session_component *new_session =
			new (md_alloc()) Log_session_component(_env, _md_alloc, _ep, label_buf, _bootstrap_phase);

	Genode::Lock::Guard guard(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Log_root::_destroy_session(Log_session_component *session)
{
	if(verbose_debug) Genode::log("Log_root::\033[33m", __func__, "\033[0m(ptr=", session,")");

	// Remove and destroy list element
	_session_rpc_objs.remove(session);
	destroy(_md_alloc, session);
}


Log_root::Log_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep, bool &bootstrap_phase)
:
	Root_component<Log_session_component>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_objs_lock        (),
	_session_rpc_objs ()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Log_root::~Log_root()
{

    while(Log_session_component *obj = _session_rpc_objs.first())
    {
    	_session_rpc_objs.remove(obj);
        Genode::destroy(_md_alloc, obj);
    }

    if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}
