/*
 * \brief  Random program for testing Genode's functionalities
 * \author Denis Huber
 * \date   2016-08-13
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <cpu_thread/client.h>

/* Rtcr includes */
#include <util/general.h>

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
	/*
	addr_t x0 = ~0UL;
	addr_t x1 = ~0UL;
	addr_t x2 = ~0UL;
	addr_t x3 = ~0UL;
	*/
	/*
	Cpu_thread_client client { main_thread->cap() };
	Thread_state ts0 { client.state() };
	log("Thread_state 0");
	Rtcr::dump_mem(((void*)ts0.sp-0x30), 0x60);
	*/
	//Rtcr::print_thread_state(ts0);
	/*
	log("utcb");
	Rtcr::dump_mem(((void*)ts.utcb)+0x600, 0x100);
	Rtcr::dump_mem(((void*)ts.utcb)+0x600, 0x100);
	*/

	log("Random ended");
}
