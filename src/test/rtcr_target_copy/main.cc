/*
 * \brief  Unit test for target_copy
 * \author Denis Huber
 * \date   2016-09-12
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <base/sleep.h>
#include <timer_session/connection.h>

/* Rtcr includes */
#include "../../rtcr/target_copy.h"
//#include "../../rtcr/util/debug.h"

using namespace Genode;

namespace Rtcr {
	struct Main;
}

struct Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 16*1024 };
	Genode::Env              &env;
	Genode::Heap              md_heap;
	Genode::Service_registry  parent_services;
	Timer::Connection         timer;

	Main(Genode::Env &env_)
	:
		env             (env_),
		md_heap         (env.ram(), env.rm()),
		parent_services (),
		timer           (env)
	{
		Target_child child { env, md_heap, parent_services, "sheep_counter", false };
		Target_copy copy { env, md_heap, child };

		timer.msleep(1000);

		child.pause();
		copy.checkpoint();
		//log("Original  Address space\n", child.pd().address_space_component().attached_regions());
		//log("Copied Address space\n", copy.copied_address_space_regions());
		child.resume();


		sleep_forever();
	}
};

size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
