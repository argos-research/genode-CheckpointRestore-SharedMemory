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
#include <base/child.h>
//#include <base/internal/elf.h>
#include <../src/include/base/internal/elf.h>
//#include <../src/include/base/internal/elf_format.h>
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

		Target_child child { env, heap, parent_services, "sheepcount", child.GRANULARITY_REDUNDANT_MEMORY};
		child.start();

		Target_state ts(env, heap);
		Checkpointer ckpt(heap, child, ts);
		timer.msleep(1000);
		ckpt.activate_redundant_memory();

		while (1) {

			timer.msleep(3000);

			ckpt.checkpoint();

			//child.resume();

		}

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
