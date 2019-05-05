/*
 * \brief  Intercepting Cpu thread
 * \author Denis Huber
 * \date   2016-10-21
 */

#include "cpu_thread_component.h"

using namespace Rtcr;


Cpu_thread_component::Cpu_thread_component(Genode::Allocator &md_alloc, Genode::Capability<Genode::Cpu_thread> cpu_thread_cap,
		Genode::Pd_session_capability pd_session_cap, const char *name, Genode::Cpu_session::Weight weight, Genode::addr_t utcb,
		Genode::Affinity::Location affinity, bool &bootstrap_phase, Genode::Rpc_entrypoint &ep)
:
	_md_alloc          (md_alloc),
	_parent_cpu_thread (cpu_thread_cap),
	_parent_state      (pd_session_cap, name, weight, utcb, bootstrap_phase)
{
	ep.manage(this);
	_parent_state.affinity = affinity;
	//if(verbose_debug) Genode::log("\033[33m", "Thread", "\033[0m<\033[35m", _parent_state.name, "\033[0m>(parent ", _parent_cpu_thread,")");
}

Cpu_thread_component::~Cpu_thread_component()
{
	//if(verbose_debug) Genode::log("\033[33m", "~Thread", "\033[0m<\033[35m", _parent_state.name, "\033[0m> ", _parent_cpu_thread);
}

Cpu_thread_component *Cpu_thread_component::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Cpu_thread_component *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}

Cpu_thread_component *Cpu_thread_component::find_by_name(const char* name)
{
	if(!Genode::strcmp(name, _parent_state.name.string()))
		return this;
	Cpu_thread_component *obj = next();
	return obj ? obj->find_by_name(name) : 0;
}


Genode::Dataspace_capability Cpu_thread_component::utcb()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.utcb();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

void Cpu_thread_component::start(Genode::addr_t ip, Genode::addr_t sp)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m(ip=",
			Genode::Hex(ip), ", sp=", Genode::Hex(sp), ")");
	_parent_cpu_thread.start(ip, sp);
	_parent_state.started = true;

}

void Cpu_thread_component::pause()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m()");
	_parent_cpu_thread.pause();
	_parent_state.paused = true;
}

void Cpu_thread_component::resume()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m()");
	_parent_cpu_thread.resume();
	_parent_state.paused = false;
}

void Cpu_thread_component::cancel_blocking()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m()");
	_parent_cpu_thread.cancel_blocking();
}

Genode::Thread_state Cpu_thread_component::state()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.state();
	if(verbose_debug) Genode::log("  result: state<kcap=",
			Genode::Hex(result.kcap), ", id=", result.id, ">");

	return result;
}

void Cpu_thread_component::state(const Genode::Thread_state& state)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m(state<kcap=",
			Genode::Hex(state.kcap), ", id=", state.id, ">)");
	_parent_cpu_thread.state(state);
}

void Cpu_thread_component::exception_sigh(Genode::Signal_context_capability handler)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m(sigh=", handler, ")");
	_parent_cpu_thread.exception_sigh(handler);
	_parent_state.sigh = handler;
}

void Cpu_thread_component::single_step(bool enabled)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m(", enabled, ")");
	_parent_cpu_thread.single_step(enabled);
	_parent_state.single_step = enabled;
}

void Cpu_thread_component::affinity(Genode::Affinity::Location location)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m(loc<pos=",
			location.xpos(), "x", location.ypos(), ", dim=",
			location.width(), "x", location.height(), ")");
	_parent_cpu_thread.affinity(location);
	_parent_state.affinity = location;
}

unsigned Cpu_thread_component::trace_control_index()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_control_index();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

Genode::Dataspace_capability Cpu_thread_component::trace_buffer()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_buffer();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

Genode::Dataspace_capability Cpu_thread_component::trace_policy()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _parent_state.name, "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_policy();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}
