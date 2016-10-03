/*
 * \brief  Test program which just counts sheeps
 * \author Denis Huber
 * \date   2016-08-04
 *
 * This program is a target for the rtcr service. It counts a sheep,
 * prints the number of the sheep and goes to sleep between each 
 * iteration. This component will be checkpointed serialized, 
 * (transfered), deserialized, and restored. It does not know that it is
 * being checkpointed.
 */

#include <base/log.h>
#include <base/component.h>
#include <timer_session/connection.h>
#include <rm_session/connection.h>
#include <region_map/client.h>

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	using namespace Genode;
	log("Creating Timer session.");
	Timer::Connection timer(env);

	log("Allocating and attaching memory and its dataspace.");
	unsigned int *addr = env.rm().attach(env.ram().alloc(4096));
	addr[0] = 1;
	unsigned int &n = *addr;

	while(1)
	{
		if(n == 1)
			log("1 sheep. zzZ");
		else
			log(n, " sheeps. zzZ");
		n++;
		timer.msleep(1000);
	}


}
