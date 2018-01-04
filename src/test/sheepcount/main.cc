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

//namespace Fiasco {
//#include <l4/sys/kdebug.h>
//}
enum { MANAGED_ADDR = 0x10000000 };

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("before restore");
	using namespace Genode;
	log("Creating Timer session.");
	Timer::Connection timer(env);


	timer.msleep(3000);
	log("Allocating and attaching memory and its dataspace.");
	Dataspace_capability ds_cap = env.ram().alloc(4096);
	unsigned int *addr = env.rm().attach(ds_cap);
//    unsigned int counter = 0;
//	unsigned int *addr = &counter;
//	unsigned int *addr = (unsigned int*) MANAGED_ADDR;
	addr[0] = 1;
	unsigned int &n = addr[0];

	//env.parent().upgrade(timer, "ram_quota=8K");
	//env.parent().upgrade(env.ram_session_cap(), "ram_quota=24K");

	while(1)
	{
        log(n, " sheep. zzZ");
		n++;
		timer.msleep(1000);
	}

}
