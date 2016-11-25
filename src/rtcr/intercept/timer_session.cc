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

	// Create virtual session object
	Timer_session_component *new_session =
			new (md_alloc()) Timer_session_component(_env, _md_alloc, _ep, args, _bootstrap_phase);

	Genode::Lock::Guard guard(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Timer_root::_destroy_session(Timer_session_component *session)
{
	if(verbose_debug) Genode::log("Timer_root::\033[33m", __func__, "\033[0m(ptr=", session,")");

	// Remove and destroy list element
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
