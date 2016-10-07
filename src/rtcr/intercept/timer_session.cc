/*
 * \brief  Intercepting timer session
 * \author Denis Huber
 * \date   2016-10-05
 */

#include "timer_session.h"

using namespace Rtcr;


Timer_session_component::Timer_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *args)
:
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_timer (env)
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Timer_session_component::~Timer_session_component()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
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
			new (md_alloc()) Timer_session_component(_env, _md_alloc, _ep, args);

	// Create and insert list element
	Timer_session_info *new_session_info =
			new (md_alloc()) Timer_session_info(*new_session, args);
	Genode::Lock::Guard guard(_infos_lock);
	_session_infos.insert(new_session_info);

	return new_session;
}


void Timer_root::_destroy_session(Timer_session_component *session)
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


Timer_root::Timer_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep)
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


Timer_root::~Timer_root()
{
    Timer_session_info *info = nullptr;

    while((info = _session_infos.first()))
    {
        _session_infos.remove(info);
        Genode::destroy(_md_alloc, info);
    }

    if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}
