/*
 * \brief  Rtcr manager
 * \author Denis Huber
 * \date   2016-08-04
 */

/* Genode include */
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>

/* Rtcr includes */
#include "target_child.h"
#include "rtcr_root.h"

namespace Rtcr {
	struct Main;
}

struct Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 16*1024 };
	Genode::Env             &env;
	Genode::Heap             md_heap         { env.ram(), env.rm() };
	Genode::Service_registry parent_services { };
	Genode::Entrypoint       root_ep         { env, ROOT_STACK_SIZE, "rtcr_root_ep" };
	Rtcr::Root               root            { root_ep, md_heap };

	Main(Genode::Env &env_) : env(env_)
	{
		//Target_child child { env, md_heap, parent_services, "sheep_counter" };
		env.parent().announce(root_ep.manage(root));

		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
