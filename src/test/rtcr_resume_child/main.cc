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
	class Main;
}

class Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 16*1024 };
	Genode::Env              &env;
	Genode::Heap              heap            { env.ram(), env.rm() };
	Genode::Service_registry  parent_services { };

public:
	Main(Genode::Env &env_) : env(env_)
	{
		using namespace Genode;
		Timer::Connection timer { env };

		log("Creating Target_child instance");
		Target_child target_child_restored { env, heap, parent_services, "sheep_counter", 0 };
		log("Starting new child");
		target_child_restored.start();

		timer.msleep(3000);

		log("target_child_restored.pause();");
		target_child_restored.pause();

		timer.msleep(3000);

		log("target_child_restored.resume();");
		target_child_restored.resume();

		log("Main: End");
		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
