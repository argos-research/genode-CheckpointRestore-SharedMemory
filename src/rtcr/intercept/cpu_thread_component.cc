/*
 * \brief  Intercepting Cpu thread
 * \author Denis Huber
 * \date   2016-10-21
 */

#include "cpu_thread_component.h"

using namespace Rtcr;

Cpu_thread_component::Cpu_thread_component(Genode::Entrypoint& ep,
		Genode::Capability<Genode::Cpu_thread> cpu_th_cap, Genode::Cpu_session::Name name)
:
	_ep(ep),
	_parent_cpu_thread(cpu_th_cap),
	_name(name)
{
	_ep.manage(*this);
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m");
}

Cpu_thread_component::~Cpu_thread_component()
{
	_ep.dissolve(*this);
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m");
}

Genode::Dataspace_capability Cpu_thread_component::utcb()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.utcb();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

void Cpu_thread_component::start(Genode::addr_t ip, Genode::addr_t sp)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m(ip=",
			Genode::Hex(ip), ", sp=", Genode::Hex(sp), ")");
	_parent_cpu_thread.start(ip, sp);
}

void Cpu_thread_component::pause()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m()");
	_parent_cpu_thread.pause();
}

void Cpu_thread_component::resume()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m()");
	_parent_cpu_thread.resume();
}

void Cpu_thread_component::cancel_blocking()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m()");
	_parent_cpu_thread.cancel_blocking();
}

Genode::Thread_state Cpu_thread_component::state()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.state();
	if(verbose_debug) Genode::log("  result: state<kcap=",
			Genode::Hex(result.kcap), ", id=", result.id, ">");

	return result;
}

void Cpu_thread_component::state(const Genode::Thread_state& state)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m(state<kcap=",
			Genode::Hex(state.kcap), ", id=", state.id, ">)");
	_parent_cpu_thread.state(state);
}

void Cpu_thread_component::exception_sigh(
		Genode::Signal_context_capability handler)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m(sigh=", handler, ")");
	_parent_cpu_thread.exception_sigh(handler);
}

void Cpu_thread_component::single_step(bool enabled)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m(", enabled, ")");
	_parent_cpu_thread.single_step(enabled);
}

void Cpu_thread_component::affinity(Genode::Affinity::Location location)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m(loc<pos=",
			location.xpos(), "x", location.ypos(), ", dim=",
			location.width(), "x", location.height(), ")");
	_parent_cpu_thread.affinity(location);
}

unsigned Cpu_thread_component::trace_control_index()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_control_index();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

Genode::Dataspace_capability Cpu_thread_component::trace_buffer()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_buffer();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

Genode::Dataspace_capability Cpu_thread_component::trace_policy()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", _name.string(), "\033[0m>::\033[33m", __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_policy();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}
