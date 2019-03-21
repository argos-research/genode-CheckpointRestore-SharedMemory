/*
 * \brief  Intercepting timer session
 * \author Denis Huber
 * \date   2016-10-05
 */

#include "timer_session.h"

using namespace Rtcr;


Timer_session_component::Timer_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
		const char *creation_args, bool bootstrapped)
:
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_timer (env),
	_parent_state (creation_args, bootstrapped)
{
	if(verbose_debug) Genode::log("\033[33m", "Timer", "\033[0m(parent ", _parent_timer, ")");
}


Timer_session_component::~Timer_session_component()
{
	if(verbose_debug) Genode::log("\033[33m", "~Timer", "\033[0m ", _parent_timer);
}


Timer_session_component *Timer_session_component::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Timer_session_component *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


void Timer_session_component::trigger_once(unsigned us)
{
	if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(us=", us, ")");
	_parent_state.timeout = us;
	_parent_state.periodic = false;
	_parent_timer.trigger_once(us);
}


void Timer_session_component::trigger_periodic(unsigned us)
{
	if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(us=", us, ")");
	_parent_state.timeout = us;
	_parent_state.periodic = true;

	_parent_timer.trigger_periodic(us);
}


void Timer_session_component::sigh(Genode::Signal_context_capability sigh)
{
	if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(", sigh, ")");

	_parent_state.sigh = sigh;

	_parent_timer.sigh(sigh);
}


unsigned long Timer_session_component::elapsed_ms() const
{
	if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m()");
	auto result = _parent_timer.elapsed_ms();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


unsigned long Timer_session_component::now_us() const
{
	if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m()");
	auto result = _parent_timer.now_us();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


void Timer_session_component::msleep(unsigned ms)
{
	if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(ms=", ms, ")");
	_parent_state.timeout = 1000*ms;
	_parent_state.periodic = false;
	_parent_timer.msleep(ms);
}


void Timer_session_component::usleep(unsigned us)
{
	if(verbose_debug) Genode::log("Timer::\033[33m", __func__, "\033[0m(us=", us, ")");
	_parent_state.timeout = us;
	_parent_state.periodic = false;
	_parent_timer.usleep(us);
}


Timer_session_component *Timer_root::_create_session(const char *args)
{
	if(verbose_debug) Genode::log("Timer_root::\033[33m", __func__, "\033[0m(", args,")");

	// Revert ram_quota calculation, because the monitor needs the original session creation argument
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Timer_session_component) + md_alloc()->overhead(sizeof(Timer_session_component));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);

	// Create virtual session object
	Timer_session_component *new_session =
			new (md_alloc()) Timer_session_component(_env, _md_alloc, _ep, readjusted_args, _bootstrap_phase);

	Genode::Lock::Guard guard(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Timer_root::_upgrade_session(Timer_session_component *session, const char *upgrade_args)
{
	if(verbose_debug) Genode::log("Timer_root::\033[33m", __func__, "\033[0m(session ", session->cap(),", args=", upgrade_args,")");

	char ram_quota_buf[32];
	char new_upgrade_args[160];

	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
}


void Timer_root::_destroy_session(Timer_session_component *session)
{
	if(verbose_debug) Genode::log("Timer_root::\033[33m", __func__, "\033[0m(session ", session->cap(),")");

	_session_rpc_objs.remove(session);
	destroy(_md_alloc, session);

}


Timer_root::Timer_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep, bool &bootstrap_phase)
:
	Root_component<Timer_session_component>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_objs_lock        (),
	_session_rpc_objs ()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Timer_root::~Timer_root()
{
    while(Timer_session_component *obj = _session_rpc_objs.first())
    {
    	_session_rpc_objs.remove(obj);
        Genode::destroy(_md_alloc, obj);
    }

    if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}
