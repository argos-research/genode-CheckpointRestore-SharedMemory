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

namespace Rtcr {
	class Cpu_session_component;
	using namespace Genode;
}

class Rtcr::Cpu_session_component : public Rpc_object<Cpu_session>
{
private:
	static constexpr bool verbose = true;

	Entrypoint &_ep;
	Allocator &_md_alloc;
	Pd_session_capability _parent_pd_cap;
	/**
	 * Parent cpu connection, usually from core
	 */
	Cpu_connection _cpu;

public:

	Cpu_session_component(char const *label, Entrypoint &ep, Allocator &md_alloc,
			Pd_session_capability parent_pd_cap)
	:
		_ep(ep), _md_alloc(md_alloc), _parent_pd_cap(parent_pd_cap), _cpu()
	{ }

	~Cpu_session_component()
	{ }

	/***************************
	 ** Cpu_session interface **
	 ***************************/

	Thread_capability create_thread(Capability<Pd_session> /* pd_cap */,
			Name const &name, Affinity::Location affinity, Weight weight,
			addr_t utcb) override
	{
		if(verbose) log("create_thread()");
		/**
		 * Note: Use physical core PD instead of virtualized Pd session
		 */
		return _cpu.create_thread(_parent_pd_cap, name, affinity, weight, utcb);
	}

	void kill_thread(Thread_capability thread) override
	{
		if(verbose) log("kill_thread()");
		_cpu.kill_thread(thread);
	}

	void exception_sigh(Signal_context_capability handler) override
	{
		if(verbose) log("exception_sigh()");
		_cpu.exception_sigh(handler);
	}

	Affinity::Space affinity_space() const override
	{
		if(verbose) log("affinity_space()");
		return _cpu.affinity_space();
	}

	Dataspace_capability trace_control() override
	{
		if(verbose) log("trace_control()");
		return _cpu.trace_control();
	}

	Quota quota() override
	{
		if(verbose) log("quota()");
		return _cpu.quota();
	}

	int ref_account(Cpu_session_capability c) override
	{
		if(verbose) log("ref_accout()");
		return _cpu.ref_account(c);
	}

	int transfer_quota(Cpu_session_capability c, size_t q) override
	{
		if(verbose) log("transfer_quota()");
		return _cpu.transfer_quota(c, q);
	}

	Capability<Native_cpu> native_cpu() override
	{
		if(verbose) log("native_cpu()");
		return _cpu.native_cpu();
	}
};

#endif /* _RTCR_CPU_SESSION_COMPONENT_H_ */
