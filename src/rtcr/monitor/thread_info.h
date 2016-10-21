/*
 * \brief  Monitoring thread information
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_THREAD_INFO_COMPONENT_H_
#define _RTCR_THREAD_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <util/string.h>
#include <thread/capability.h>
#include <cpu_thread/client.h>

/* Rtcr includes */
#include "../intercept/cpu_thread_component.h"

namespace Rtcr {
	struct Thread_info;
}

/**
 * Struct which holds a thread capability which belong to the client
 */
struct Rtcr::Thread_info : Genode::List<Thread_info>::Element
{
	Cpu_thread_component        &cpu_thread;
	Genode::Cpu_session::Name    name;
	Genode::Affinity::Location   affinity;
	Genode::Cpu_session::Weight  weight;
	Genode::addr_t               utcb;

	/**
	 * Constructor
	 */
	Thread_info(Cpu_thread_component &cpu_thread, Genode::Cpu_session::Name name,
			Genode::Affinity::Location affinity, Genode::Cpu_session::Weight weight,
			Genode::addr_t utcb)
	:
		cpu_thread (cpu_thread),
		name       (name),
		affinity   (affinity),
		weight     (weight),
		utcb       (utcb)
	{ }

	Thread_info *find_by_cap(Genode::Thread_capability cap)
	{
		if(cap == cpu_thread.cap())
			return this;
		Thread_info *thread_info = next();
		return thread_info ? thread_info->find_by_cap(cap) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::Cpu_thread_client thread_client (cpu_thread.cap());
		Genode::Thread_state ts (thread_client.state());

		Genode::print(output, "Thread ", name.string(), "\n");
		Genode::print(output, "r0-r4: ", Hex(ts.r0, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r1, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r2, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r3, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r4, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "r5-r9: ", Hex(ts.r5, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r6, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r7, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r8, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r9, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "r10-r12: ", Hex(ts.r10, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r11, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r12, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "sp, lr, ip, cpsr, cpu_e: ", Hex(ts.sp, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.lr, Hex::PREFIX, Hex::PAD), " ", Hex(ts.ip, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.cpsr, Hex::PREFIX, Hex::PAD), " ", Hex(ts.cpu_exception, Hex::PREFIX, Hex::PAD), "\n");
	}
};

#endif /* _RTCR_THREAD_INFO_COMPONENT_H_ */
