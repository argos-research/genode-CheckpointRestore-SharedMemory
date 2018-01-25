/*
 * \brief  Rtcr child creation
 * \author Denis Huber
 * \date   2016-08-26
 */

/* Genode include */
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>
#include <timer_session/connection.h>

/* Rtcr includes */
#include "../../rtcr/target_child.h"
#include "../../rtcr/target_state.h"
#include "../../rtcr/checkpointer.h"
#include "../../rtcr/restorer.h"

namespace Rtcr {
struct Main;
}

struct Rtcr::Main {
	enum {
		ROOT_STACK_SIZE = 16 * 1024
	};
	Genode::Env &env;
	Genode::Heap heap { env.ram(), env.rm() };
	Genode::Service_registry parent_services { };

	Main(Genode::Env &env_) :
			env(env_) {
		using namespace Genode;

		Timer::Connection timer { env };

		Target_child child { env, heap, parent_services, "sheepcount", 1 };
		child.start();

		Target_state ts(env, heap);
		Checkpointer ckpt(heap, child, ts);

		while (1) {
			timer.msleep(5000);
			//child.pause();
			//ckpt.checkpoint();
			//timer.msleep(1000);

			//if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m()");

			// Pause all threads of all sessions

			//resume() seems not reliable, thus call it multiple times

			child.ram().parent_state().ram_dataspaces.first()->mrm_info->dd_infos.first()->detach();
			timer.msleep(1000);
			// Iterate through every session
			Cpu_session_component *cpu_session = &child.cpu();
			//child._custom_services.cpu_root->session_infos().first();
			while (cpu_session) {

				// Iterate through every CPU thread
				Cpu_thread_component *cpu_thread =
						cpu_session->parent_state().cpu_threads.first();
				int j = 0;
				while (cpu_thread) {
					// Pause the CPU thread
					//Genode::Cpu_thread_client client{cpu_thread->parent_cap()};
					//client.pause();
					//if(cpu_thread->state().unresolved_page_fault)
					{

						PINF("Thread: %i, Pagefault: %i, IP: %lx", j++,
								cpu_thread->state().unresolved_page_fault,
								cpu_thread->state().ip);
						Genode::Thread_state st = cpu_thread->state();
						//st.ip += 2;
						cpu_thread->state(st);

					}
					cpu_thread = cpu_thread->next();
				}

				cpu_session = cpu_session->next();
			}

			// for(int i=0;i<10;i++)
			// 	child.resume();
		}

//      timer.msleep(3000);       
//		ckpt.checkpoint();

//		Target_child child_restored { env, heap, parent_services, "sheep_counter", 0 };
//		Restorer resto(heap, child_restored, ts);
//		child_restored.start(resto);

		//log("The End");
		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() {
	return 32 * 1024;
}

void Component::construct(Genode::Env &env) {
	static Rtcr::Main main(env);
}
