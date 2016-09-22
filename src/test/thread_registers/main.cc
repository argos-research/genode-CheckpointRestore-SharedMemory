/*
 * \brief  Reading the state of a thread to two specific times
 * \author Denis Huber
 * \date   2016-08-13
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <cpu_thread/client.h>
#include <base/semaphore.h>

void print_thread_state(Genode::Thread_state &ts, bool brief = false)
{
	using namespace Genode;

	if(!brief)
	{
		log("  r0:   ", Hex(ts.r0,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r0                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r1:   ", Hex(ts.r1,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r1                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r2:   ", Hex(ts.r2,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r2                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r3:   ", Hex(ts.r3,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r3                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r4:   ", Hex(ts.r4,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r4                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r5:   ", Hex(ts.r5,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r5                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r6:   ", Hex(ts.r6,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r6                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r7:   ", Hex(ts.r7,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r7                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r8:   ", Hex(ts.r8,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r8                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r9:   ", Hex(ts.r9,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r9                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r10:  ", Hex(ts.r10,           Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r10                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r11:  ", Hex(ts.r11,           Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r11                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r12:  ", Hex(ts.r12,           Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r12                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  sp:   ", Hex(ts.sp,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.sp                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  lr:   ", Hex(ts.lr,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.lr                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  ip:   ", Hex(ts.ip,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.ip                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  cpsr: ", Hex(ts.cpsr,          Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.cpsr                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  ce:   ", Hex(ts.cpu_exception, Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.cpu_exception         - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  upf:  ", ts.unresolved_page_fault?"true ":"false", "     ", "  pos: ", Hex((addr_t)&ts.unresolved_page_fault - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  exc:  ", ts.exception?"true ":"false",             "     ", "  pos: ", Hex((addr_t)&ts.exception             - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  kcap: ", Hex(ts.kcap,          Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.kcap                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  id:   ", Hex(ts.id,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.id                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  utcb: ", Hex(ts.utcb,          Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.utcb                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  excs: ", Hex(ts.exceptions,    Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.exceptions            - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  psd:  ", ts.paused?"true ":"false",                "     ", "  pos: ", Hex((addr_t)&ts.paused                - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  iexc: ", ts.in_exception?"true ":"false",          "     ", "  pos: ", Hex((addr_t)&ts.in_exception          - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  lock: ", "/",                                  "         ", "  pos: ", Hex((addr_t)&ts.lock                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	}
	else
	{
		log("   r0-r4:  ", Hex(ts.r0,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r1,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r2,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r3, Hex::PREFIX, Hex::PAD),  "  ", Hex(ts.r4, Hex::PREFIX, Hex::PAD));
		log("   r5-r9:  ", Hex(ts.r5,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r6,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r7,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r8, Hex::PREFIX, Hex::PAD),  "  ", Hex(ts.r9, Hex::PREFIX, Hex::PAD));
		log("  r10-r12: ", Hex(ts.r10, Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r11, Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r12, Hex::PREFIX, Hex::PAD));
		log("  ip, sp:  ", Hex(ts.ip,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.sp,  Hex::PREFIX, Hex::PAD));
	}
}

using namespace Genode;

struct Child_thread : Thread
{
	volatile unsigned long &phase;

	Child_thread(Env &env, volatile unsigned long &phase)
	:
		Thread(env, "test_thread", 0x2000),
		phase(phase)
	{ }

	void entry()
	{
		while(phase != 0) ;
		log("test_thread: Phase 0 (phase=", phase, ")");
		// Here the first sample is taken
		phase++;

		while(phase != 2) ;
		log("test_thread: Phase 2 (phase=", phase, ")");
		// Here the second sample is taken
		// increase stack pointer (?)
		do_phase2();

		while(phase != 4) ;
		log("test_thread: Phase 4 (phase=", phase, ")");

	}

	void do_phase2()
	{
		long dummy = 0;
		log("test_thread: Created dummy=", dummy," in phase ", phase);
		phase++;
		while(phase != 4) dummy = 1;
	}
};


size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- thread_registers started ---");
	volatile unsigned long phase = 0;
	Child_thread thread0 { env, phase };
	thread0.start();
	Cpu_thread_client client { thread0.cap() };

	while(phase != 1) ;
	log("main_thread: Phase 1 (phase=", phase, ")");
	client.pause();
	log("main_thread: Taking first sample");
	Thread_state ts0 { client.state() };
	client.resume();
	phase++;

	while(phase != 3) ;
	log("main_thread: Phase 3 (phase=", phase, ")");
	client.pause();
	log("main_thread: Taking second sample");
	Thread_state ts1 { client.state() };
	client.resume();
	phase++;


	thread0.join();
	log("First sample");
	print_thread_state(ts0, true);
	log("Second sample");
	print_thread_state(ts1, true);

	log("--- thread_registers ended ---");
}
