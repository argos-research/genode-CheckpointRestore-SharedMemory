/*
 * \brief  Intercepting Cpu thread
 * \author Denis Huber
 * \date   2016-10-21
 */

#ifndef _RTCR_CPU_THREAD_COMPONENT_H_
#define _RTCR_CPU_THREAD_COMPONENT_H_

/* Genode inlcudes */
#include <cpu_thread/client.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <base/entrypoint.h>

/* Rtcr includes */
#include "../monitor/cpu_thread_info.h"

namespace Rtcr {
	class Cpu_thread_component;

	constexpr bool cpu_thread_verbose_debug = false;
}

class Rtcr::Cpu_thread_component : public Genode::Rpc_object<Genode::Cpu_thread>,
                                   public Genode::List<Cpu_thread_component>::Element
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = cpu_thread_verbose_debug;

	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator         &_md_alloc;
	/**
	 * Name of the thread
	 */
	const char *               _name;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Cpu_thread_client  _parent_cpu_thread;
	/**
	 * State of parent's RPC object
	 */
	Cpu_thread_info            _parent_state;

public:

	Cpu_thread_component(Genode::Allocator &md_alloc, Genode::Capability<Genode::Cpu_thread> cpu_thread_cap,
			const char *name, Genode::Cpu_session::Weight weight, Genode::addr_t utcb, Genode::Affinity::Location affinity,
			bool &bootstrap_phase);
	~Cpu_thread_component();

	Genode::Capability<Genode::Cpu_thread> parent_cap() { return _parent_cpu_thread; }

	Cpu_thread_info &parent_state() { return _parent_state; }
	Cpu_thread_info const &parent_state() const { return _parent_state; }

	Cpu_thread_component *find_by_badge(Genode::uint16_t badge);

	/******************************
	 ** Cpu thread Rpc interface **
	 ******************************/

	Genode::Dataspace_capability utcb                () override;
	void                         start               (Genode::addr_t ip, Genode::addr_t sp) override;
	void                         pause               () override;
	void                         resume              () override;
	void                         cancel_blocking     () override;
	Genode::Thread_state         state               () override;
	void                         state               (Genode::Thread_state const &state) override;
	void                         exception_sigh      (Genode::Signal_context_capability handler) override;
	void                         single_step         (bool enabled) override;
	void                         affinity            (Genode::Affinity::Location location) override;
	unsigned                     trace_control_index () override;
	Genode::Dataspace_capability trace_buffer        () override;
	Genode::Dataspace_capability trace_policy        () override;
};

#endif /* _RTCR_CPU_THREAD_COMPONENT_H_ */
