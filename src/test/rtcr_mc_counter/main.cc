/*
 * \brief  Testprogram which just counts sheeps
 * \author Denis Huber
 * \date   2016-08-04
 *
 * This program is a target for the rtcr service. It counts a sheep,
 * prints the number of the sheep and goes to sleep between each 
 * iteration. This component will be checkpointed serialized, 
 * (transfered), deserialized, and restored. It does not know that it is
 * being checkpointed.
 */

#include <base/component.h>
#include <timer_session/connection.h>
#include <base/log.h>
#include <rm_session/connection.h>
#include <region_map/client.h>

namespace Fiasco {
#include <l4/sys/kdebug.h>
}

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("before restore");
	using namespace Genode;
	log("Creating Timer session.");
	Timer::Connection timer(env);

	log("Allocating and attaching memory and its dataspace.");
	Dataspace_capability ds_cap = env.ram().alloc(4096);

	char *addr = env.rm().attach(ds_cap);

	addr[0] = 'H';
	addr[1] = 'A';
	addr[2] = 'L';
	addr[3] = 'L';
	addr[4] = 'O';

	log("Dataspace attached at \033[31m", &addr,"\033[0m");
	log("First Integer: \033[31m", addr[0],"\033[0m");
	log("Second Integer: \033[31m", addr[1],"\033[0m");
	log("Third Integer: \033[31m", addr[2],"\033[0m");

	//env.parent().upgrade(timer, "ram_quota=8K");
	//env.parent().upgrade(env.ram_session_cap(), "ram_quota=24K");

	timer.msleep(20000);


}
