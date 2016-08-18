/*
 * \brief Intercepting Pd session
 * \author Denis Huber
 * \date 2016-08-10
 */

#ifndef _RTCR_CPU_SESSION_COMPONENT_H_
#define _RTCR_CPU_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <cpu_session/connection.h>
#include <cpu_thread/client.h>

namespace Rtcr {
	class Cpu_session_component;
}

class Rtcr::Cpu_session_component : public Genode::Rpc_object<Genode::Cpu_session>
{
private:
	static constexpr bool verbose_debug = true;

	Genode::Env       &_env;
	Genode::Allocator &_md_alloc;
	/**
	 * Parent pd session, usually from core
	 */
	Genode::Pd_session_capability _parent_pd_cap;
	/**
	 * Connection to parent's cpu session, usually from core
	 */
	Genode::Cpu_connection _parent_cpu;

	struct Thread_info : Genode::List<Thread_info>::Element
	{
		Genode::Thread_capability thread_cap;

		Thread_info(Genode::Thread_capability thread_cap)
		: thread_cap(thread_cap) { }

		Thread_info *find_by_cap(Genode::Thread_capability cap)
		{
			if(thread_cap.local_name() == cap.local_name())
				return this;
			Thread_info *thread_info = next();
			return thread_info ? thread_info->find_by_cap(cap) : 0;
		}

	};

	Genode::Lock _threads_lock;
	Genode::List<Thread_info> _threads;

public:

	Cpu_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Pd_session_capability parent_pd_cap, const char *name)
	:
		_env          (env),
		_md_alloc     (md_alloc),
		_parent_pd_cap(parent_pd_cap),
		_parent_cpu   (env, name)
	{
		if(verbose_debug) Genode::log("Cpu_session_component created");
	}

	~Cpu_session_component()
	{
		if(verbose_debug) Genode::log("Cpu_session_component destroyed");
	}

	Genode::Cpu_session_capability parent_cap()
	{
		return _parent_cpu.cap();
	}

	/***************************
	 ** Cpu_session interface **
	 ***************************/

	Genode::Thread_capability create_thread(Genode::Pd_session_capability /* pd_cap */,
			Name const &name, Genode::Affinity::Location affinity, Weight weight,
			Genode::addr_t utcb) override
	{
		if(verbose_debug) Genode::log("Cpu::create_thread()\n  Name: ", name.string());

		/**
		 * Note: Use physical core PD instead of virtualized Pd session
		 */
		Genode::Thread_capability thread_cap = _parent_cpu.create_thread(_parent_pd_cap, name, affinity, weight, utcb);

		// Store the thread
		Genode::Lock::Guard _lock_guard(_threads_lock);
		_threads.insert(new (_md_alloc) Thread_info(thread_cap));
/*
		if(!Genode::strcmp(name.string(), "Worker3"))
		{
			Genode::Thread_state state = Genode::Cpu_thread_client(_threads.first()->thread_cap).state();
			Genode::log("Worker1: ip ", state.ip, ", sp ", state.sp);
		}
*/
/*
		Genode::size_t i = 0;
		Thread_info *curr = _threads.first();
		while(curr)
		{
			i++;
			curr = curr->next();
		}
		Genode::log("There are ", i, " threads stored");
*/
		return thread_cap;
	}

	void kill_thread(Genode::Thread_capability thread) override
	{
		if(verbose_debug) Genode::log("Cpu::kill_thread()");

		// Find thread
		Genode::Lock::Guard lock_guard(_threads_lock);
		Thread_info *thread_info = _threads.first()->find_by_cap(thread);
		if(!thread_info)
		{
			Genode::error("Thread with capability ", thread.local_name(), " (local) not found!");
			return;
		}

		// Remove and destroy thread from list and allocator
		_threads.remove(thread_info);
		destroy(_md_alloc, thread_info);

		_parent_cpu.kill_thread(thread);
	}

	void exception_sigh(Genode::Signal_context_capability handler) override
	{
		if(verbose_debug) Genode::log("Cpu::exception_sigh()");
		_parent_cpu.exception_sigh(handler);
	}

	Genode::Affinity::Space affinity_space() const override
	{
		if(verbose_debug) Genode::log("Cpu::affinity_space()");
		return _parent_cpu.affinity_space();
	}

	Genode::Dataspace_capability trace_control() override
	{
		if(verbose_debug) Genode::log("Cpu::trace_control()");
		return _parent_cpu.trace_control();;
	}

	Quota quota() override
	{
		if(verbose_debug) Genode::log("Cpu::quota()");
		return _parent_cpu.quota();
	}

	int ref_account(Genode::Cpu_session_capability c) override
	{
		if(verbose_debug) Genode::log("Cpu::ref_accout()");
		return _parent_cpu.ref_account(c);
	}

	int transfer_quota(Genode::Cpu_session_capability c, Genode::size_t q) override
	{
		if(verbose_debug) Genode::log("Cpu::transfer_quota()");
		return _parent_cpu.transfer_quota(c, q);
	}

	Genode::Capability<Native_cpu> native_cpu() override
	{
		if(verbose_debug) Genode::log("Cpu::native_cpu()");
		return _parent_cpu.native_cpu();
	}
};

#endif /* _RTCR_CPU_SESSION_COMPONENT_H_ */
