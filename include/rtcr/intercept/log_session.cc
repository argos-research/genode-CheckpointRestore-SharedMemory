/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \date   2016-10-02
 */

#include "log_session.h"

using namespace Rtcr;


Log_session_component::Log_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
		const char *label, const char *creation_args, bool bootstrapped, Resources resources, Diag diag)
:
	Session_object(ep, resources, label, diag),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_log   (env, label),
	_parent_state (creation_args, bootstrapped)
{
	//if(verbose_debug) Genode::log("\033[33m", "Log", "\033[0m(parent ", _parent_log,")");
}


Log_session_component::~Log_session_component()
{
	//if(verbose_debug) Genode::log("\033[33m", "~Log", "\033[0m ", _parent_log);
}


Log_session_component *Log_session_component::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Log_session_component *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
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

	// Revert ram_quota calculation, because the monitor needs the original session creation argument
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Log_session_component) + md_alloc()->overhead(sizeof(Log_session_component));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);

	Genode::Session::Diag diag{};
	// Create virtual session object
	Log_session_component *new_session =
			new (md_alloc()) Log_session_component(_env, _md_alloc, _ep, label_buf, readjusted_args, _bootstrap_phase, Genode::session_resources_from_args(readjusted_args), diag);

	Genode::Lock::Guard guard(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Log_root::_upgrade_session(Log_session_component *session, const char *upgrade_args)
{
	if(verbose_debug) Genode::log("Log_root::\033[33m", __func__, "\033[0m(session ", session->cap(),", args=", upgrade_args,")");

	char ram_quota_buf[32];
	char new_upgrade_args[160];

	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(Genode::Parent::Env::log(), upgrade_args);
}


void Log_root::_destroy_session(Log_session_component *session)
{
	if(verbose_debug) Genode::log("Log_root::\033[33m", __func__, "\033[0m(ptr=", session,")");

	// Remove and destroy list element
	_session_rpc_objs.remove(session);
	destroy(*session);
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
