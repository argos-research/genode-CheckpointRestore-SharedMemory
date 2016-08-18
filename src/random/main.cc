/*
 * \brief  Random program for testing Genode's functionalities
 * \author Denis Huber
 * \date   2016-08-13
 */

#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <cpu_thread/client.h>

#include <util/general.h>
/*namespace Fiasco {
#include <l4/sys/thread.h>
}*/

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	Thread *main_thread = Thread::myself();
	/*log("Thread");
	log("  n:  ", main_thread->name().string());
	log("  vb: ", Hex(main_thread->stack_area_virtual_base()));
	log("  vs: ", Hex(main_thread->stack_area_virtual_size()));
	log("  pb: ", main_thread->stack_base());
	log("  pt: ", main_thread->stack_top());*/
	/*Native_thread &nt = main_thread->native_thread();
	log("Native_thread");
	log("  kcap: ", nt.kcap);*/
	/*Native_utcb *nu = main_thread->utcb();
	log("Native_utcb");
	log("  utcb: ", (void*)nu->foc_utcb);*/
	/*Cpu_thread_client client { main_thread->cap() };
	Thread_state ts { client.state() };
	log("Thread_state");
	log("  r0:   ", ts.r0,   ", pos: ", Hex((addr_t)&ts.r0   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r1:   ", ts.r1,   ", pos: ", Hex((addr_t)&ts.r1   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r2:   ", ts.r2,   ", pos: ", Hex((addr_t)&ts.r2   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r3:   ", ts.r3,   ", pos: ", Hex((addr_t)&ts.r3   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r4:   ", ts.r4,   ", pos: ", Hex((addr_t)&ts.r4   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r5:   ", ts.r5,   ", pos: ", Hex((addr_t)&ts.r5   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r6:   ", ts.r6,   ", pos: ", Hex((addr_t)&ts.r6   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r7:   ", ts.r7,   ", pos: ", Hex((addr_t)&ts.r7   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r8:   ", ts.r8,   ", pos: ", Hex((addr_t)&ts.r8   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r9:   ", ts.r9,   ", pos: ", Hex((addr_t)&ts.r9   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r10:  ", ts.r10,  ", pos: ", Hex((addr_t)&ts.r10  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r11:  ", ts.r11,  ", pos: ", Hex((addr_t)&ts.r11  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  r12:  ", ts.r12,  ", pos: ", Hex((addr_t)&ts.r12  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  sp:   ", Hex(ts.sp),   ", pos: ", Hex((addr_t)&ts.sp   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  lr:   ", ts.lr,   ", pos: ", Hex((addr_t)&ts.lr   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  ip:   ", Hex(ts.ip),   ", pos: ", Hex((addr_t)&ts.ip   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  cpsr: ", ts.cpsr, ", pos: ", Hex((addr_t)&ts.cpsr - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  ce:   ", ts.cpu_exception, ", pos: ", Hex((addr_t)&ts.cpu_exception - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  upf:  ", ts.unresolved_page_fault?"true":"false", ", pos: ", Hex((addr_t)&ts.unresolved_page_fault - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  exc:  ", ts.exception?"true":"false", ", pos: ", Hex((addr_t)&ts.exception - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  kcap: ", Hex(ts.kcap, Hex::PREFIX, Hex::PAD), ", pos: ", Hex((addr_t)&ts.kcap - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  id:   ", ts.id,   ", pos: ", Hex((addr_t)&ts.id   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  utcb: ", Hex(ts.utcb, Hex::PREFIX, Hex::PAD), ", pos: ", Hex((addr_t)&ts.utcb - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  excs: ", ts.exceptions, ", pos: ", Hex((addr_t)&ts.exceptions - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  psd:  ", ts.paused?"true":"false", ", pos: ", Hex((addr_t)&ts.paused - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  iexc: ", ts.in_exception?"true":"false", ", pos: ", Hex((addr_t)&ts.in_exception - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	log("  exc:  ", "/", ", pos: ", Hex((addr_t)&ts.lock - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	Rtcr::dump_mem(&ts, sizeof(ts));
	log("Size");
	log("  Cpu_state:         ", Hex(sizeof(Cpu_state), Hex::PREFIX, Hex::PAD));
	log("  Thread_state_base: ", Hex(sizeof(Thread_state_base), Hex::PREFIX, Hex::PAD));
	log("  Thread_state:      ", Hex(sizeof(Thread_state), Hex::PREFIX, Hex::PAD));*/

	Cpu_thread_client client { main_thread->cap() };
	Thread_state ts0 { client.state() };
	Thread_state ts1 { client.state() };
	log("Thread_state 0");
	log("  sp:   ", Hex(ts0.sp),   ", pos: ", Hex((addr_t)&ts0.sp   - (addr_t)&ts0, Hex::PREFIX, Hex::PAD));
	log("  ip:   ", Hex(ts0.ip),   ", pos: ", Hex((addr_t)&ts0.ip   - (addr_t)&ts0, Hex::PREFIX, Hex::PAD));
	log("Thread_state 1");
	log("  sp:   ", Hex(ts1.sp),   ", pos: ", Hex((addr_t)&ts1.sp   - (addr_t)&ts1, Hex::PREFIX, Hex::PAD));
	log("  ip:   ", Hex(ts1.ip),   ", pos: ", Hex((addr_t)&ts1.ip   - (addr_t)&ts1, Hex::PREFIX, Hex::PAD));
	Thread_state ts2 { client.state() };
	log("Thread_state 2");
	log("  sp:   ", Hex(ts2.sp),   ", pos: ", Hex((addr_t)&ts2.sp   - (addr_t)&ts2, Hex::PREFIX, Hex::PAD));
	log("  ip:   ", Hex(ts2.ip),   ", pos: ", Hex((addr_t)&ts2.ip   - (addr_t)&ts2, Hex::PREFIX, Hex::PAD));
	/*
	log("utcb");
	Rtcr::dump_mem(((void*)ts.utcb)+0x600, 0x100);
	Rtcr::dump_mem(((void*)ts.utcb)+0x600, 0x100);
	*/

	log("Random ended");
}
