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

namespace Rtcr {
	class Cpu_session_component;
}

class Rtcr::Cpu_session_component : public Genode::Rpc_object<Genode::Cpu_session>
{
private:
	static constexpr bool verbose = false;

	Genode::Entrypoint &_ep;
	Genode::Allocator  &_md_alloc;
	/**
	 * Parent pd session, usually from core
	 */
	Genode::Pd_session_capability _parent_pd_cap;
	/**
	 * Connection to parent's cpu session, usually from core
	 */
	Genode::Cpu_connection _parent_cpu;

public:

	Cpu_session_component(Genode::Entrypoint &ep, Genode::Allocator &md_alloc, Genode::Pd_session_capability parent_pd_cap)
	:
		_ep(ep), _md_alloc(md_alloc),
		_parent_pd_cap(parent_pd_cap),
		_parent_cpu()
	{
		_ep.manage(*this);
		if(verbose)
		{
			Genode::log("Cpu_session_component created");
			//log("Arguments: env=", &ep, ", md_alloc=", &md_alloc, ", parent_pd_cap=", parent_pd_cap.local_name());
			//log("State: _ep=", &_ep, ", _md_alloc=", &_md_alloc, ", _parent_pd_cap=", _parent_pd_cap.local_name(),
			//		", _parent_cpu=", _parent_cpu.local_name());
		}
	}

	~Cpu_session_component()
	{
		_ep.dissolve(*this);
		if(verbose) Genode::log("Cpu_session_component destroyed");
	}

	Genode::Cpu_session_capability parent_cpu_cap()
	{
		return _parent_cpu.cap();
	}

	/***************************
	 ** Cpu_session interface **
	 ***************************/

	Genode::Thread_capability create_thread(Genode::Capability<Genode::Pd_session> /* pd_cap */,
			Name const &name, Genode::Affinity::Location affinity, Weight weight,
			Genode::addr_t utcb) override
	{
		if(verbose) Genode::log("Cpu::create_thread()");
		/**
		 * Note: Use physical core PD instead of virtualized Pd session
		 */
		return _parent_cpu.create_thread(_parent_pd_cap, name, affinity, weight, utcb);
	}

	void kill_thread(Genode::Thread_capability thread) override
	{
		if(verbose) Genode::log("Cpu::kill_thread()");
		_parent_cpu.kill_thread(thread);
	}

	void exception_sigh(Genode::Signal_context_capability handler) override
	{
		if(verbose) Genode::log("Cpu::exception_sigh()");
		_parent_cpu.exception_sigh(handler);
	}

	Genode::Affinity::Space affinity_space() const override
	{
		if(verbose) Genode::log("Cpu::affinity_space()");
		return _parent_cpu.affinity_space();
	}

	Genode::Dataspace_capability trace_control() override
	{
		if(verbose) Genode::log("Cpu::trace_control()");
		Genode::Dataspace_capability ds_cap = _parent_cpu.trace_control();
		//Genode::log("  ds_cap: ", ds_cap.local_name(), ", valid: ", ds_cap.valid()?"true":"false");
		return ds_cap;
	}

	Quota quota() override
	{
		if(verbose) Genode::log("Cpu::quota()");
		return _parent_cpu.quota();
	}

	int ref_account(Genode::Cpu_session_capability c) override
	{
		if(verbose) Genode::log("Cpu::ref_accout()");
		return _parent_cpu.ref_account(c);
	}

	int transfer_quota(Genode::Cpu_session_capability c, Genode::size_t q) override
	{
		if(verbose) Genode::log("Cpu::transfer_quota()");
		return _parent_cpu.transfer_quota(c, q);
	}

	Genode::Capability<Native_cpu> native_cpu() override
	{
		if(verbose) Genode::log("Cpu::native_cpu()");
		return _parent_cpu.native_cpu();
	}
};

#endif /* _RTCR_CPU_SESSION_COMPONENT_H_ */
