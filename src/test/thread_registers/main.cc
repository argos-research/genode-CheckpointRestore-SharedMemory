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

/* Rtcr includes */
#include <util/general.h>

using namespace Genode;

namespace Random
{
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
}

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- thread_registers started ---");
	volatile unsigned long phase = 0;
	Random::Child_thread thread0 { env, phase };
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
	Rtcr::print_thread_state(ts0, true);
	log("Second sample");
	Rtcr::print_thread_state(ts1, true);

	log("--- thread_registers ended ---");
}
