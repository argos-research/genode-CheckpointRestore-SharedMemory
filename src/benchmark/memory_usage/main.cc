/*
 * \brief  Testprogram which allocates and writes memory.
 * \author David Werner
 * \date   2019-01-04
 *
 * This program is a target for the rtcr service. It is able to allocate a preset
 * amount of memory in either one or multiple dataspaces. Aftwards the dataspaces
 * are filled with arbitrary content.
 */

#include <base/component.h>
#include <timer_session/connection.h>
#include <base/log.h>
#include <rm_session/connection.h>
#include <region_map/client.h>

namespace Fiasco {
#include <l4/sys/kdebug.h>
}

// Profiling include
#include <util/profiler.h>

#define MEM 600*1024*1024
#define TEST_MEM_LIMIT 128*1024*1024;
#define NUM_DS 1

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("debug mem usage benchmark");
	Genode::log("Creating Timer session.");
	Timer::Connection timer(env);
	

	Genode::log("Allocating and attaching dataspaces");

	Genode::size_t available = env.ram().avail();

	Genode::log("Available mem: ", available);

	Genode::addr_t addrs[NUM_DS];

	//{
		//PROFILE_SCOPE("memory_usage", "lightblue", timer);

		for(int i=0; i<NUM_DS; i++) addrs[i] = 0;

		Genode::size_t mem = TEST_MEM_LIMIT;
		Genode::size_t ds_size = mem/NUM_DS;
		//Genode::log("DS size: ", ds_size);
		for(int i=0; i<NUM_DS; i++) {
			Genode::Dataspace_capability ds = env.ram().alloc(ds_size);
			addrs[i] = env.rm().attach(ds);
		}
	//}

	Genode::size_t available2 = env.ram().avail();
	Genode::log("Available mem: ", available2);
	Genode::log("Mem difference: ", available - available2);
	Genode::log("Adresse 1: ", addrs[0]);
	//Genode::log("Adresse 2: ", addrs[1]);

	int content = 0;

	while(1) {
		//Genode::log("Output: ", was);
		for(int i=0; i< NUM_DS; i++) {
			addrs[i] = content;
			content = (content+1)%1024;
		}
	}

	Genode::log("--- test ended ---");
}
