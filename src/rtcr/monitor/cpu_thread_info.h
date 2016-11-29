/*
 * \brief  Stores CPU thread state
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_THREAD_INFO_H_
#define _RTCR_THREAD_INFO_H_

/* Genode includes */
#include <cpu_session/cpu_session.h>
#include <pd_session/capability.h>
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
	Genode::Pd_session_capability const pd_session_cap;
	Genode::Cpu_session::Name     const name;
	Genode::Cpu_session::Weight   const weight;
	Genode::addr_t                const utcb;

	// Modifiable state
	bool started;
	bool paused;
	bool single_step;
	Genode::Affinity::Location        affinity; // Is also used for creation
	Genode::Signal_context_capability sigh;

	Cpu_thread_info(Genode::Pd_session_capability pd_session_cap, const char* name, Genode::Cpu_session::Weight weight,
			Genode::addr_t utcb, bool bootstrapped)
	:
		Normal_rpc_info (bootstrapped),
		pd_session_cap(pd_session_cap),
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

		Genode::print(output, "pd_session ", pd_session_cap, ", name=", name, ", weigth=", weight.value, ", utcb=", Hex(utcb), ", ");
		Genode::print(output, "started=", started, ", paused=", paused, ", single_step=", single_step, ", ");
		Genode::print(output, "affinity=(", affinity.xpos(), "x", affinity.ypos(), ", ", affinity.width(), "x", affinity.height());
		Genode::print(output, "), sigh ", sigh, ", ");
		Normal_rpc_info::print(output);

/*
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
				Hex(ts.cpsr, Hex::PREFIX, Hex::PAD), " ", Hex(ts.cpu_exception, Hex::PREFIX, Hex::PAD));
*/
	}
};

#endif /* _RTCR_THREAD_INFO_COMPONENT_H_ */
