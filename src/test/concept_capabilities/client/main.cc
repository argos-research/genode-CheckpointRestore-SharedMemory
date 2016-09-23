/*
 * \brief  Client which cap space will be manipulated
 * \author Denis Huber
 * \date   2016-09-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/sleep.h>
#include <base/component.h>
#include <timer_session/connection.h>

/* Resource includes */
#include <resource_session/connection.h>

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("Initialization started");
	Timer::Connection timer {env};

	log("Requesting session to Resource service");
	Resource::Connection resource_service {env};

	log("Sending native pd cap");
	resource_service.provide(env.pd().native_pd());

	log("My native pd cap: ", env.pd().native_pd());

	sleep_forever();
}
