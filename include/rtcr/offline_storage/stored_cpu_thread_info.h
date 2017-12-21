/*
 * \brief  Structure for storing thread information
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_STORED_THREAD_INFO_H_
#define _RTCR_STORED_THREAD_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../intercept/cpu_thread_component.h"
#include "../offline_storage/stored_info_structs.h"

namespace Rtcr {
	struct Stored_cpu_thread_info;
}


struct Rtcr::Stored_cpu_thread_info : Stored_normal_info, Genode::List<Stored_cpu_thread_info>::Element
{
	Genode::uint16_t            const pd_session_badge;
	Genode::Cpu_session::Name   const name;
	Genode::Cpu_session::Weight const weight;
	Genode::addr_t              const utcb;
	bool started;
	bool paused;
	bool single_step;
	Genode::Affinity::Location affinity;
	Genode::uint16_t           sigh_badge;
	Genode::Thread_state       ts;

	Stored_cpu_thread_info(Cpu_thread_component &cpu_thread, Genode::addr_t targets_kcap)
	:
		Stored_normal_info(targets_kcap,
				cpu_thread.cap().local_name(),
				cpu_thread.parent_state().bootstrapped),
		pd_session_badge(cpu_thread.parent_state().pd_session_cap.local_name()),
		name        	(cpu_thread.parent_state().name),
		weight      	(cpu_thread.parent_state().weight),
		utcb        	(cpu_thread.parent_state().utcb),
		started     	(cpu_thread.parent_state().started),
		paused      	(cpu_thread.parent_state().paused),
		single_step 	(cpu_thread.parent_state().single_step),
		affinity    	(cpu_thread.parent_state().affinity),
		sigh_badge  	(cpu_thread.parent_state().sigh.local_name()),
		ts          	()
	{ }

	Stored_cpu_thread_info(Genode::uint16_t _pd_session_badge, Genode::Cpu_session::Name _name, 
		Genode::Cpu_session::Weight _weight, Genode::addr_t _utcb, bool _started, bool _paused, 
		bool _single_step, Genode::Affinity::Location _affinity, Genode::uint16_t _sigh_badge)
	:
		Stored_normal_info(0,0,false),
		pd_session_badge(_pd_session_badge),
		name        	(_name),
		weight      	(_weight),
		utcb        	(_utcb),
		started     	(_started),
		paused      	(_paused),
		single_step 	(_single_step),
		affinity    	(_affinity),
		sigh_badge  	(_sigh_badge),
		ts          	()
	{ }

	Stored_cpu_thread_info *find_by_name(const char *name)
	{
		if(!Genode::strcmp(name, this->name.string()))
			return this;
		Stored_cpu_thread_info *info = next();
		return info ? info->find_by_name(name) : 0;
	}

	Stored_cpu_thread_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_cpu_thread_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_normal_info::print(output);
		Genode::print(output, ", pd_session_badge=", pd_session_badge, ", name=", name, ", weight=", weight.value, ", utcb=", Hex(utcb));
		Genode::print(output, ", started=", started, ", paused=", paused, ", single_step=", single_step);
		Genode::print(output, ", affinity=(", affinity.xpos(), "x", affinity.ypos(),
				", ", affinity.width(), "x", affinity.height(), ")");
		Genode::print(output, ", sigh_badge=", sigh_badge, "\n");

		Genode::print(output, "  r0-r4: ", Hex(ts.r0, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r1, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r2, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r3, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r4, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "  r5-r9: ", Hex(ts.r5, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r6, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r7, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r8, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r9, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "  r10-r12: ", Hex(ts.r10, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.r11, Hex::PREFIX, Hex::PAD), " ", Hex(ts.r12, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "  sp, lr, ip, cpsr, cpu_e: ", Hex(ts.sp, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.lr, Hex::PREFIX, Hex::PAD), " ", Hex(ts.ip, Hex::PREFIX, Hex::PAD), " ",
				Hex(ts.cpsr, Hex::PREFIX, Hex::PAD), " ", Hex(ts.cpu_exception, Hex::PREFIX, Hex::PAD));
	}

};

#endif /* _RTCR_STORED_THREAD_INFO_H_ */
