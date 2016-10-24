/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \date   2016-08-10
 */

#include "cpu_session_component.h"

using namespace Rtcr;


void Cpu_session_component::_destroy(Thread_info* info)
{
	// Remove thread info from list
	_threads.remove(info);

	// Destroy virtual cpu thread and thread info
	Genode::destroy(_md_alloc, &info->cpu_thread);
	Genode::destroy(_md_alloc, info);
}


Cpu_session_component::Cpu_session_component(
		Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
		Genode::Pd_session_capability parent_pd_cap, bool &phase_restore, const char *name)
:
	_env           (env),
	_md_alloc      (md_alloc),
	_ep            (ep),
	_parent_pd_cap (parent_pd_cap),
	_parent_cpu    (env, name),
	_phase_restore (phase_restore),
	_threads_lock  (),
	_threads       ()

{
	if(verbose_debug) Genode::log("\033[33m", "Cpu", "\033[0m(parent ", _parent_cpu,")");
}


Cpu_session_component::~Cpu_session_component()
{
	while(Thread_info *thread_info = _threads.first())
	{
		_destroy(thread_info);
	}

	if(verbose_debug) Genode::log("\033[33m", "~Cpu", "\033[0m ", _parent_cpu);
}


void Cpu_session_component::pause_threads()
{
	for(Thread_info *curr_th = _threads.first(); curr_th; curr_th = curr_th->next())
	{
		Genode::Cpu_thread_client{curr_th->cpu_thread.parent_cap()}.pause();
	}
}


void Cpu_session_component::resume_threads()
{
	for(Thread_info *curr_th = _threads.first(); curr_th; curr_th = curr_th->next())
	{
		Genode::Cpu_thread_client{curr_th->cpu_thread.parent_cap()}.resume();
	}
}


Genode::Thread_capability Cpu_session_component::create_thread(Genode::Pd_session_capability /* pd_cap */,
		Name const &name, Genode::Affinity::Location affinity, Weight weight,
		Genode::addr_t utcb)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(name=", name.string(), ")");

	// Note: Use parent's Pd session instead of virtualized Pd session
	Genode::Thread_capability thread_cap = _parent_cpu.create_thread(_parent_pd_cap, name, affinity, weight, utcb);

	// Create virtual Cpu_thread and its management list element
	Cpu_thread_component *new_cpu_thread = new (_md_alloc) Cpu_thread_component(_ep, thread_cap, _phase_restore, name);
	Thread_info *new_th_info = new (_md_alloc) Thread_info(*new_cpu_thread, name, affinity,weight, utcb);

	// Store the thread
	Genode::Lock::Guard _lock_guard(_threads_lock);
	_threads.insert(new_th_info);

	if(verbose_debug) Genode::log("  Created virtual thread ", new_th_info->cpu_thread.cap());

	return new_th_info->cpu_thread.cap();
}


void Cpu_session_component::kill_thread(Genode::Thread_capability thread)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(", thread,")");

	Genode::Lock::Guard lock_guard(_threads_lock);

	// Find thread
	Thread_info *thread_info = _threads.first()->find_by_cap(thread);
	if(!thread_info)
	{
		Genode::error("Thread ", thread, " not found!");
		return;
	}

	_destroy(thread_info);

	_parent_cpu.kill_thread(thread);
}

void Cpu_session_component::exception_sigh(Genode::Signal_context_capability handler)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(", handler, ")");

	_parent_state.exception_sigh = handler;
	_parent_cpu.exception_sigh(handler);
}

Genode::Affinity::Space Cpu_session_component::affinity_space() const
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m()");

	auto result = _parent_cpu.affinity_space();

	if(verbose_debug) Genode::log("  result: ", result.width(), "x", result.height(), " (", result.total(), ")");

	return result;
}

Genode::Dataspace_capability Cpu_session_component::trace_control()
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m()");

	auto result = _parent_cpu.trace_control();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

Genode::Cpu_session::Quota Cpu_session_component::quota()
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m()");

	auto result = _parent_cpu.quota();

	if(verbose_debug) Genode::log("  result: super_period_us=", result.super_period_us, ", us=", result.us);

	return result;
}

int Cpu_session_component::ref_account(Genode::Cpu_session_capability c)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(", c, ")");

	auto result = _parent_cpu.ref_account(c);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

int Cpu_session_component::transfer_quota(Genode::Cpu_session_capability c, Genode::size_t q)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(to ", c, "quota=", q, ")");

	auto result = _parent_cpu.transfer_quota(c, q);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}

Genode::Capability<Genode::Cpu_session::Native_cpu> Cpu_session_component::native_cpu()
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m()");

	auto result = _parent_cpu.native_cpu();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}
