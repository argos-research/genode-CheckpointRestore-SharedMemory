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
	struct Copied_thread_info;
}

/**
 * Struct which holds a thread capability which belong to the client
 */
struct Rtcr::Copied_thread_info : Genode::List<Copied_thread_info>::Element
{
	Genode::String<Genode::Cpu_session::THREAD_NAME_LEN> name;
	Genode::addr_t r0, r1, r2, r3, r4, r5, r6,
			r7, r8, r9, r10, r11, r12;        /* general purpose register 0..12 */
	Genode::addr_t sp;                        /* stack pointer */
	Genode::addr_t lr;                        /* link register */
	Genode::addr_t ip;                        /* instruction pointer */
	Genode::addr_t cpsr;                      /* current program status register */
	Genode::addr_t cpu_exception;             /* last hardware exception */

	/**
	 * Constructor
	 */
	Copied_thread_info(Genode::Cpu_session::Name<Genode::Cpu_session::THREAD_NAME_LEN> name,
			Genode::addr_t r0, Genode::addr_t r1, Genode::addr_t r2, Genode::addr_t r3, Genode::addr_t r4,
			Genode::addr_t r5, Genode::addr_t r6, Genode::addr_t r7, Genode::addr_t r8, Genode::addr_t r9,
			Genode::addr_t r10, Genode::addr_t r11, Genode::addr_t r12, Genode::addr_t sp, Genode::addr_t lr,
			Genode::addr_t ip, Genode::addr_t cpsr, Genode::addr_t cpu_exception)
	:
		name(name),
		r0(r0), r1(r1), r2(r2), r3(r3), r4(r4), r5(r5), r6(r6), r7(r7), r8(r8), r9(r9), r10(r10), r11(r11), r12(r12),
		sp(sp), lr(lr), ip(ip), cpsr(cpsr), cpu_exception(cpu_exception)
	{ }

	Copied_thread_info *find_by_name(const char *name)
	{
		if(!Genode::strcmp(name, this->name.string()))
			return this;
		Copied_thread_info *thread_info = next();
		return thread_info ? thread_info->find_by_name(name) : 0;
	}

};

#endif /* _RTCR_COPIED_THREAD_INFO_COMPONENT_H_ */
