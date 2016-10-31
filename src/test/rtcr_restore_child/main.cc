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

namespace Rtcr {
	struct Main;
}

struct Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 16*1024 };
	Genode::Env              &env;
	Genode::Heap              heap            { env.ram(), env.rm() };
	Genode::Service_registry  parent_services { };

	Main(Genode::Env &env_) : env(env_)
	{
		using namespace Genode;

		Timer::Connection timer { env };

		Target_child child { env, heap, parent_services, "sheep_counter", 0 };
		child.start();

		timer.msleep(3000);

		Target_state ts(env, heap);
		Checkpointer ckpt(heap, child, ts);
		ckpt.checkpoint();


		log("The End");
		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
