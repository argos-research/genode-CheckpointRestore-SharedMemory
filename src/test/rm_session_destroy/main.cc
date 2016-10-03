/*
 * \brief  Rm_session destroy test
 * \author Denis Huber
 * \date   2016-10-03
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <rm_session/connection.h>

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- Rm_session::destroy test started ---");

	log("Creating connection to core's Rm_session");
	Rm_connection rm(env);

	log("Creating new region map");
	Capability<Region_map> cap = rm.create(4096);

	log("Destroying new region map");
	rm.destroy(cap);

	log("--- Rm_session::destroy test ended ---");
}
