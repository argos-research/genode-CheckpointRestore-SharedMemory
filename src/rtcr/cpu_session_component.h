/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \date   2016-08-10
 */

#ifndef _RTCR_CPU_SESSION_COMPONENT_H_
#define _RTCR_CPU_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <cpu_session/connection.h>
#include <cpu_thread/client.h>

namespace Rtcr {
	struct Thread_info;
	class Cpu_session_component;
}

/**
 * Struct which holds a thread capability which belong to the client
 */
struct Rtcr::Thread_info : Genode::List<Rtcr::Thread_info>::Element
{
	/**
	 * Capability of the thread
	 */
	Genode::Thread_capability thread_cap;

	/**
	 * Constructor
	 *
	 * \param thread_cap Capability of the thread
	 */
	Thread_info(Genode::Thread_capability thread_cap)
	: thread_cap(thread_cap) { }

	/**
	 * Find Thread_info by using a specific Thread_capability
	 *
	 * \param cap Thread_capability
	 *
	 * \return Thread_info with the corresponding Capability
	 */
	Thread_info *find_by_cap(Genode::Thread_capability cap)
	{
		if(thread_cap.local_name() == cap.local_name())
			return this;
		Thread_info *thread_info = next();
		return thread_info ? thread_info->find_by_cap(cap) : 0;
	}

};

/**
 * This custom Cpu session intercepts the creation and destruction of threads by the client
 */
class Rtcr::Cpu_session_component : public Genode::Rpc_object<Genode::Cpu_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = false;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env                   &_env;
	/**
	 * Allocator for objects belonging to the monitoring of threads (e.g. Thread_info)
	 */
	Genode::Allocator             &_md_alloc;
	/**
	 * Parent Pd session, usually from core; used for creating a thread
	 */
	Genode::Pd_session_capability  _parent_pd_cap;
	/**
	 * Connection to parent's Cpu session, usually from core; this class wraps this session
	 */
	Genode::Cpu_connection         _parent_cpu;
	/**
	 * Lock to make _threads thread-safe
	 */
	Genode::Lock                   _threads_lock;
	/**
	 * List of client's thread capabilities
	 */
	Genode::List<Thread_info>      _threads;

public:

	/**
	 * Constructor
	 *
	 * \param env           Environment for creating a session to parent's Cpu service
	 * \param md_alloc      Allocator for Thread_info
	 * \param parent_pd_cap Capability to parent's Pd session for creating new threads
	 * \param name          Label for parent's Cpu session
	 */
	Cpu_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Pd_session_capability parent_pd_cap, const char *name)
	:
		_env          (env),
		_md_alloc     (md_alloc),
		_parent_pd_cap(parent_pd_cap),
		_parent_cpu   (env, name),
		_threads_lock (),
		_threads      ()
	{
		if(verbose_debug) Genode::log("Cpu_session_component created");
	}

	/**
	 * Destructor
	 */
	~Cpu_session_component()
	{
		//TODO free allocator from Thread_info objects
		if(verbose_debug) Genode::log("Cpu_session_component destroyed");
	}

	/**
	 * Return parent's Cpu session capability
	 *
	 * \return Parent's Cpu session capability
	 */
	Genode::Cpu_session_capability parent_cap()
	{
		return _parent_cpu.cap();
	}

	/**
	 * Create a copy of threads list containing thread capabilities.
	 * Is needed, if the client's threads are not paused.
	 *
	 * \param alloc Allocator where the new list shall be stored
	 *
	 * \return new list
	 */
	Genode::List<Thread_info> copy_threads_list(Genode::Allocator &alloc)
	{
		// Serialize access to the list
		Genode::Lock::Guard lock(_threads_lock);

		Genode::List<Thread_info> result;

		// Store all Thread_infos
		Thread_info *curr_th = _threads.first();
		for(; curr_th; curr_th = curr_th->next())
		{
			// Create a copy of Thread_info and attach it to result
			result.insert(new (alloc) Thread_info(curr_th->thread_cap));
		}

		return result;
	}

	/**
	 * Return list of client's threads
	 *
	 * \return Reference to the internal threads list
	 */
	Genode::List<Thread_info> &threads()
	{
		return _threads;
	}

	/**
	 * Pause all threads
	 */
	void pause_threads()
	{
		Thread_info *curr_th = _threads.first();
		for( ; curr_th; curr_th = curr_th->next())
		{
			Genode::Cpu_thread_client{curr_th->thread_cap}.pause();
		}
	}

	/**
	 * Resume all threads
	 */
	void resume_threads()
	{
		Thread_info *curr_th = _threads.first();
		for( ; curr_th; curr_th = curr_th->next())
		{
			Genode::Cpu_thread_client{curr_th->thread_cap}.resume();
		}
	}

	/***************************
	 ** Cpu_session interface **
	 ***************************/

	/**
	 * Create a thread and store its capability in Thread_info
	 */
	Genode::Thread_capability create_thread(Genode::Pd_session_capability /* pd_cap */,
			Name const &name, Genode::Affinity::Location affinity, Weight weight,
			Genode::addr_t utcb) override
	{
		if(verbose_debug)
		{
			Genode::log("Cpu::create_thread(name=", name.string(), ")");
		}

		/**
		 * Note: Use parent's Pd session instead of virtualized Pd session
		 */
		Genode::Thread_capability thread_cap = _parent_cpu.create_thread(_parent_pd_cap, name, affinity, weight, utcb);

		// Store the thread
		Genode::Lock::Guard _lock_guard(_threads_lock);
		_threads.insert(new (_md_alloc) Thread_info(thread_cap));

		if(verbose_debug)
		{
			Genode::log("  thread_cap=", thread_cap.local_name());
		}

		return thread_cap;
	}

	/**
	 * Destroy thread and its Thread_info
	 */
	void kill_thread(Genode::Thread_capability thread) override
	{
		if(verbose_debug) Genode::log("Cpu::kill_thread(cap=", thread.local_name(),")");

		Genode::Lock::Guard lock_guard(_threads_lock);

		// Find thread
		Thread_info *thread_info = _threads.first()->find_by_cap(thread);
		if(!thread_info)
		{
			Genode::error("Thread with capability ", thread.local_name(), " not found!");
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
