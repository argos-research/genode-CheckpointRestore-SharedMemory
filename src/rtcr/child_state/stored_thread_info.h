/*
 * \brief  Structure for copying thread information
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_COPIED_THREAD_INFO_COMPONENT_H_
#define _RTCR_COPIED_THREAD_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <thread/capability.h>
#include <cpu_session/cpu_session.h>

namespace Rtcr {
	struct Stored_thread_info;
}

/**
 * Struct which holds a thread capability which belong to the client
 */
struct Rtcr::Stored_thread_info : Genode::List<Stored_thread_info>::Element
{
	Genode::Cpu_session::Name name;
	Genode::Thread_state ts;

	Stored_thread_info(Genode::Cpu_session::Name name, Genode::Thread_state &ts)
	:
		name(name),
		ts(ts)
	{ }

	Stored_thread_info *find_by_name(const char *name)
	{
		if(!Genode::strcmp(name, this->name.string()))
			return this;
		Stored_thread_info *thread_info = next();
		return thread_info ? thread_info->find_by_name(name) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

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

#endif /* _RTCR_COPIED_THREAD_INFO_COMPONENT_H_ */
