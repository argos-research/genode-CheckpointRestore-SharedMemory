/*
 * \brief  Target component
 * \author Denis Huber
 * \date   2016-09-17
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <timer_session/connection.h>

/* Resource includes */
#include <resource_session/connection.h>

using namespace Genode;

size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	log("Initialization started");
	Timer::Connection timer {env};

	log("Requesting session to Resource service");
	Resource::Connection resource_service {env};

	log("Sending main thread cap");
	resource_service.thread(Thread::myself()->cap());

	log("Requesting dataspace cap");
	Dataspace_capability ds_cap = resource_service.dataspace();

	log("Attaching dataspace cap");
	unsigned int *addr = env.rm().attach(ds_cap);

	log("Initialization ended");

	log("Starting main loop");
	while(true)
	{
		log(*addr);
		(*addr)++;


		timer.msleep(1000);
	}
}
