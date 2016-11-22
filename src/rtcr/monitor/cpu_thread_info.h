/*
 * \brief  Stores CPU thread state
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_THREAD_INFO_H_
#define _RTCR_THREAD_INFO_H_

/* Genode includes */
#include <thread/capability.h>

/* Rtcr includes */
#include "info_structs.h"

namespace Rtcr {
	struct Cpu_thread_info;
}

/**
 * Struct which holds a thread capability which belong to the client
 */
struct Rtcr::Cpu_thread_info : Normal_rpc_info
{
	// Creation arguments
	Genode::Cpu_session::Name   const name;
	Genode::Cpu_session::Weight const weight;
	Genode::addr_t              const utcb;

	// Variable state
	bool started;
	bool paused;
	bool single_step;
	Genode::Affinity::Location        affinity;
	Genode::Signal_context_capability sigh;

	Cpu_thread_info(Genode::Cpu_session::Name name, Genode::Cpu_session::Weight weight,
			Genode::addr_t utcb, bool bootstrapped)
	:
		Normal_rpc_info (bootstrapped),
		name        (name),
		weight      (weight),
		utcb        (utcb),
		started     (false),
		paused      (false),
		single_step (false),
		affinity    (),
		sigh        ()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "name=", name.string(), " weight=", weight.value, " utcb=", Hex(utcb), ", ");
		Normal_rpc_info::print(output);
/*		Genode::print(output, "r0-r4: ", Hex(ts.r0, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r1, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r2, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r3, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r4, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "r5-r9: ", Hex(ts.r5, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r6, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r7, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r8, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r9, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "r10-r12: ", Hex(ts.r10, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r11, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r12, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "sp, lr, ip, cpsr, cpu_e: ", Hex(ts.sp, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.lr, Hex::PREFIX, Hex::PAD), " ", Hex(ts.ip, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.cpsr, Hex::PREFIX, Hex::PAD), " ", Hex(ts.cpu_exception, Hex::PREFIX, Hex::PAD));*/
	}
};

#endif /* _RTCR_THREAD_INFO_COMPONENT_H_ */
